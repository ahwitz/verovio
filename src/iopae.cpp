/////////////////////////////////////////////////////////////////////////////
// Name:        iopae.cpp
// Author:      Rodolfo Zitellini // this is the important stuff!
// Created:     2012
// Copyright (c) Laurent Pugin. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "iopae.h"

//----------------------------------------------------------------------------

#include <assert.h>
#include <sstream>
#include <string>

//----------------------------------------------------------------------------

#include "beam.h"
#include "chord.h"
#include "clef.h"
#include "doc.h"
#include "keysig.h"
#include "layer.h"
#include "measure.h"
#include "metersig.h"
#include "multirest.h"
#include "note.h"
#include "rest.h"
#include "score.h"
#include "scoredef.h"
#include "section.h"
#include "staff.h"
#include "tuplet.h"
#include "vrv.h"

//----------------------------------------------------------------------------

#ifndef NO_PAE_SUPPORT
#include <regex>
#endif

namespace vrv {

#define BEAM_INITIAL 0x01
#define BEAM_MEDIAL 0x02
#define BEAM_TERMINAL 0x04

// User interface variables:
int debugQ = 0; // used with --debug option
int stdoutQ = 0;
char outdir[1024] = { 0 }; // used with -d option
char extension[1024] = { 0 }; // used with -e option
char hum2abc[1024] = { 0 }; // used with -a option
int quietQ = 0; // used with -q option
int quiet2Q = 0; // used with -Q option

// Global variables:
char data_line[10001] = { 0 };
#define MAX_DATA_LEN 1024 // One line of the pae file would not be that long!
char data_key[MAX_DATA_LEN];
char data_value[MAX_DATA_LEN]; // ditto as above

//----------------------------------------------------------------------------
// PaeInput
//----------------------------------------------------------------------------

PaeInput::PaeInput(Doc *doc, std::string filename)
    : // This is pretty bad. We open a bad fileoinputstream as we don't use it
    FileInputStream(doc)
{
    m_filename = filename;
    m_staff = NULL;
    m_measure = NULL;
    m_layer = NULL;
    m_last_tied_note = NULL;
    m_is_in_chord = false;
}

PaeInput::~PaeInput()
{
}

//////////////////////////////////////////////////////////////////////////

bool PaeInput::ImportFile()
{
#ifndef NO_PAE_SUPPORT
    std::ifstream infile;
    infile.open(m_filename.c_str());
    parsePlainAndEasy(infile);
    return true;
#else
    LogError("Plain and Easy import is not supported in the build.");
    return false;
#endif
}

bool PaeInput::ImportString(std::string const &pae)
{
#ifndef NO_PAE_SUPPORT
    std::istringstream in_stream(pae);
    parsePlainAndEasy(in_stream);
    return true;
#else
    LogError("Plain and Easy import is not support in the build.");
    return false;
#endif
}

#ifndef NO_PAE_SUPPORT

//////////////////////////////
//
// parsePlainAndEasy --
//

void PaeInput::parsePlainAndEasy(std::istream &infile)
{
    // buffers
    char c_clef[1024] = { 0 };
    char c_key[1024] = { 0 };
    char c_keysig[1024] = { 0 };
    char c_timesig[1024] = { 0 };
    char c_alttimesig[1024] = { 0 };
    char incipit[10001] = { 0 };
    int in_beam = 0;

    std::string s_key;
    pae::Measure current_measure;
    pae::Note current_note;
    Clef *staffDefClef = NULL;
    MeterSig *scoreDefMeterSig = NULL;
    KeySig *scoreDefKeySig = NULL;

    std::vector<pae::Measure> staff;

    // read values
    while (!infile.eof()) {
        infile.getline(data_line, 10000);
        if (infile.eof()) {
            // LogDebug("Truncated file or ending tag missing");
            // exit(1);
        }
        getAtRecordKeyValue(data_key, data_value, data_line);
        if (strcmp(data_key, "end") == 0) {
            break;
        }
        else if (strcmp(data_key, "clef") == 0) {
            strcpy(c_clef, data_value);
        }
        else if (strcmp(data_key, "key") == 0) {
            strcpy(c_key, data_value);
        }
        else if (strcmp(data_key, "keysig") == 0) {
            strcpy(c_keysig, data_value);
        }
        else if (strcmp(data_key, "timesig") == 0) {
            strcpy(c_timesig, data_value);
        }
        else if (strcmp(data_key, "alttimesig") == 0) {
            strcpy(c_alttimesig, data_value);
        }
        else if (strcmp(data_key, "data") == 0) {
            strcpy(incipit, data_value);
        }
    }

    if (strlen(c_clef)) {
        Clef *c = new Clef;
        getClefInfo(c_clef, c); // do we need to put a default clef?
        if (!staffDefClef)
            staffDefClef = c;
        else
            current_measure.clef = c;
    }

    if (strlen(c_keysig)) {
        KeySig *k = new KeySig();
        getKeyInfo(c_keysig, k);
        if (!scoreDefKeySig) {
            scoreDefKeySig = k;
        }
        else {
            if (current_measure.key) {
                delete current_measure.key;
            }
            current_measure.key = k;
        }
    }
    if (strlen(c_timesig)) {
        MeterSig *meter = new MeterSig;
        getTimeInfo(c_timesig, meter);
        if (!scoreDefMeterSig) {
            scoreDefMeterSig = meter;
        }
        else {
            if (current_measure.meter) {
                delete current_measure.meter;
            }
            current_measure.meter = meter;
        }
    }

    // read the incipit string
    int length = (int)strlen(incipit);
    int i = 0;
    while (i < length) {
        // eat the input...

        if (incipit[i] == ' ') {
            // just skip
            i++;
        }

        // octaves
        if ((incipit[i] == '\'') || (incipit[i] == ',')) {
            i += getOctave(incipit, &current_note.octave, i);
        }

        // rhythmic values
        else if (isdigit(incipit[i]) != 0) {
            i += getDurations(incipit, &current_measure, i);
        }

        // accidentals (1 = n; 2 = x; 3 = xx; 4 = b; 5 = bb)
        else if (incipit[i] == 'n' || incipit[i] == 'x' || incipit[i] == 'b') {
            i += getAccidental(incipit, &current_note.accidental, i);
        }

        //
        // beaming starts
        else if (incipit[i] == '{') {
            // current_note.beam = 1;
            current_note.beam = BEAM_INITIAL;
            in_beam++;
        }

        // beaming ends
        else if (incipit[i] == '}' && in_beam > 0) {
            current_measure.notes[current_measure.notes.size() - 1].beam = BEAM_TERMINAL;
            current_note.beam = 0;
            in_beam--;
        }

        // slurs are read when adding the note
        else if (incipit[i] == '+') {
        }

        // beginning tuplets & fermatas
        else if (incipit[i] == '(') {
            i += getTupletFermata(incipit, &current_note, i);
        }

        // end of tuplets
        else if ((incipit[i] == ';') || (incipit[i] == ')')) {
            i += getTupletFermataEnd(incipit, &current_note, i);
        }

        // trills are read when adding the note
        else if (incipit[i] == 't') {
        }

        // grace notes
        else if ((incipit[i] == 'g') || (incipit[i] == 'q')) {
            i += getGraceNote(incipit, &current_note, i);
        }

        // end of appogiatura
        else if (incipit[i] == 'r') {
            current_note.appoggiatura = 0; // should not have to be done, but just in case
        }

        // note and rest
        // getNote also creates a new note object
        else if (((incipit[i] - 'A' >= 0) && (incipit[i] - 'A' < 7)) || (incipit[i] == '-')) {
            i += getNote(incipit, &current_note, &current_measure, i);
        }

        // whole rest
        else if (incipit[i] == '=') {
            i += getWholeRest(incipit, &current_measure.wholerest, i);
        }

        // abbreviation
        else if (incipit[i] == '!') {
            i += getAbbreviation(incipit, &current_measure, i);
        }

        // measure repetition
        else if ((incipit[i] == 'i') && staff.size() > 0) {
            pae::Measure last_measure = staff[staff.size() - 1];
            current_measure.notes = last_measure.notes;
            current_measure.wholerest = last_measure.wholerest;
        }

        // barLine
        else if ((incipit[i] == ':') || (incipit[i] == '/')) {
            i += getBarLine(incipit, &current_measure.barLine, i);
            current_measure.abbreviation_offset = 0; // just in case...
            staff.push_back(current_measure);
            current_measure.reset();
        }

        // clef change
        else if ((incipit[i] == '%') && (i + 1 < length)) {
            Clef *c = new Clef;
            i += getClefInfo(incipit, c, i + 1);
            //
            if (!staffDefClef) {
                staffDefClef = c;
            }
            // If there are no notes yet in the measure
            // attach this clef change to the measure
            else if (current_measure.notes.size() == 0) {
                // If a clef was already assigned, remove it
                if (current_measure.clef) delete current_measure.clef;

                current_measure.clef = c;
            }
            else {
                // as above
                if (current_note.clef) delete current_note.clef;

                current_note.clef = c;
            }
        }

        // time signature change
        else if ((incipit[i] == '@') && (i + 1 < length)) {
            MeterSig *meter = new MeterSig;
            i += getTimeInfo(incipit, meter, i + 1);
            if (current_measure.notes.size() == 0) {
                if (current_measure.meter) {
                    delete current_measure.meter;
                }
                // When will this be deleted? Potential memory leak? LP
                current_measure.meter = meter;
            }
            else {
                if (current_note.meter) {
                    delete current_note.meter;
                }
                current_note.meter = meter;
            }
        }

        // key signature change
        else if ((incipit[i] == '$') && (i + 1 < length)) {
            KeySig *k = new KeySig;
            i += getKeyInfo(incipit, k, i + 1);
            if (current_measure.notes.size() == 0) {
                if (current_measure.key) delete current_measure.key;

                current_measure.key = k;
            }
            else {
                if (current_note.key) delete current_note.key;

                current_note.key = k;
            }
        }

        i++;
    }

    // we need to add the last measure if it has no barLine at the end
    if (current_measure.notes.size() != 0) {
        // current_measure.barLine = "=-";
        staff.push_back(current_measure);
        current_measure.notes.clear();
    }

    m_doc->SetType(Raw);
    Score *score = m_doc->CreateScoreBuffer();
    // the section
    Section *section = new Section();
    score->AddChild(section);

    int measure_count = 1;

    std::vector<pae::Measure>::iterator it;
    for (it = staff.begin(); it < staff.end(); it++) {

        m_staff = new Staff(1);
        m_measure = new Measure(true, measure_count);
        m_layer = new Layer();
        m_layer->SetN(1);

        m_staff->AddChild(m_layer);
        m_measure->AddChild(m_staff);

        pae::Measure obj = *it;

        // Add a score def if we have a new key sig or meter sig
        if (obj.key || obj.meter) {
            ScoreDef *scoreDef = new ScoreDef();
            if (obj.key) {
                scoreDef->SetKeySig(obj.key->ConvertToKeySigLog());
                delete obj.key;
                obj.key = NULL;
            }
            if (obj.meter) {
                scoreDef->SetMeterCount(obj.meter->GetCount());
                scoreDef->SetMeterUnit(obj.meter->GetUnit());
                scoreDef->SetMeterSym(obj.meter->GetSym());
                delete obj.meter;
                obj.meter = NULL;
            }
            section->AddChild(scoreDef);
        }

        section->AddChild(m_measure);

        convertMeasure(&obj);
        measure_count++;
    }

    // add minimal scoreDef
    StaffGrp *staffGrp = new StaffGrp();
    StaffDef *staffDef = new StaffDef();
    staffDef->SetN(1);
    staffDef->SetLines(5);
    if (staffDefClef) {
        staffDef->SetClefShape(staffDefClef->GetShape());
        staffDef->SetClefLine(staffDefClef->GetLine());
        staffDef->SetClefDis(staffDefClef->GetDis());
        staffDef->SetClefDisPlace(staffDefClef->GetDisPlace());
        delete staffDefClef;
    }
    if (scoreDefKeySig) {
        m_doc->m_scoreDef.SetKeySig(scoreDefKeySig->ConvertToKeySigLog());
        delete scoreDefKeySig;
    }
    if (scoreDefMeterSig) {
        m_doc->m_scoreDef.SetMeterCount(scoreDefMeterSig->GetCount());
        m_doc->m_scoreDef.SetMeterUnit(scoreDefMeterSig->GetUnit());
        m_doc->m_scoreDef.SetMeterSym(scoreDefMeterSig->GetSym());
        delete scoreDefMeterSig;
    }
    staffGrp->AddChild(staffDef);
    m_doc->m_scoreDef.AddChild(staffGrp);

    m_doc->ConvertToPageBasedDoc();
}

//////////////////////////////
//
// getOctave --
//
#define BASE_OCT 4
int PaeInput::getOctave(const char *incipit, char *octave, int index)
{
    int i = index;
    int length = (int)strlen(incipit);
    if (incipit[i] == '\'') {
        *octave = BASE_OCT;
        while ((i + 1 < length) && (incipit[i + 1] == '\'')) {
            (*octave)++;
            i++;
        }
    }
    else if (incipit[i] == ',') {
        // negative octave
        *octave = BASE_OCT - 1;
        while ((i + 1 < length) && (incipit[i + 1] == ',')) {
            (*octave)--;
            i++;
        }
    }

    return i - index;
}

//////////////////////////////
//
// getDuration --
//

int PaeInput::getDuration(const char *incipit, data_DURATION *duration, int *dot, int index)
{

    int i = index;
    int length = (int)strlen(incipit);

    switch (incipit[i]) {
        case '0': *duration = DURATION_long; break;
        case '1': *duration = DURATION_1; break;
        case '2': *duration = DURATION_2; break;
        case '3': *duration = DURATION_32; break;
        case '4': *duration = DURATION_4; break;
        case '5': *duration = DURATION_64; break;
        case '6': *duration = DURATION_16; break;
        case '7': *duration = DURATION_128; break;
        case '8': *duration = DURATION_8; break;
        case '9': *duration = DURATION_breve; break;
    }

    *dot = 0;
    if ((i + 1 < length) && (incipit[i + 1] == '.')) {
        // one dot
        (*dot)++;
        i++;
    }
    if ((i + 1 < length) && (incipit[i + 1] == '.')) {
        // two dots
        (*dot)++;
        i++;
    }
    if ((*dot == 1) && (incipit[i] == 7)) {
        // neumatic notation
        *duration = DURATION_breve;
        *dot = 0;
        LogWarning("Found a note in neumatic notation (7.), using breve instead");
    }

    return i - index;
}

//////////////////////////////
//
// getDurations --
//

int PaeInput::getDurations(const char *incipit, pae::Measure *measure, int index)
{
    int i = index;
    int length = (int)strlen(incipit);

    measure->durations_offset = 0;
    measure->durations.clear();
    measure->dots.clear();

    // int j = 0;
    do {
        int dot;
        data_DURATION dur = DURATION_4;
        // measure->dots.setSize(j+1);
        i += getDuration(incipit, &dur, &dot, i);
        measure->durations.push_back(dur);
        measure->dots.push_back(dot);
        // j++;
        if ((i + 1 < length) && isdigit(incipit[i + 1])) {
            i++;
        }
        else {
            break;
        }
    } while (1);

    return i - index;
}

//////////////////////////////
//
// getAccidental --
//

int PaeInput::getAccidental(const char *incipit, data_ACCIDENTAL_EXPLICIT *accident, int index)
{
    int i = index;
    int length = (int)strlen(incipit);

    if (incipit[i] == 'n') {
        *accident = ACCIDENTAL_EXPLICIT_n;
    }
    else if (incipit[i] == 'x') {
        *accident = ACCIDENTAL_EXPLICIT_s;
        if ((i + 1 < length) && (incipit[i + 1] == 'x')) {
            *accident = ACCIDENTAL_EXPLICIT_ss;
            i++;
        }
    }
    else if (incipit[i] == 'b') {
        *accident = ACCIDENTAL_EXPLICIT_f;
        if ((i + 1 < length) && (incipit[i + 1] == 'b')) {
            *accident = ACCIDENTAL_EXPLICIT_ff;
            i++;
        }
    }
    return i - index;
}

//////////////////////////////
//
// getTupletOrFermata --
//

int PaeInput::getTupletFermata(const char *incipit, pae::Note *note, int index)
{
    int i = index;
    int length = (int)strlen(incipit);

    // Detect if it is a fermata or a tuplet.
    //
    // std::regex_constants::ECMAScript is the default syntax, so optional.
    // Previously these were extended regex syntax, but this case
    // is the same in ECMAScript syntax.
    std::regex exp("^([^)]*[ABCDEFG-][^)]*[ABCDEFG-][^)]*)", std::regex_constants::ECMAScript);
    bool is_tuplet = regex_search(incipit + i, exp);

    if (is_tuplet) {
        int t = i;
        int t2 = 0;
        int tuplet_val = 0;
        char *buf;

        // Triplets are in the form (4ABC)
        // index points to the '(', so we look back
        // if the resut is a number or dot, it means we have the long format
        // i.e. 4(6ABC;5) or 4.(6ABC;5)
        if ((index != 0) && (isdigit(incipit[index - 1]) || incipit[index - 1] == '.')) {

            // create the buffer so we can convert the tuplet nr to int
            buf = (char *)malloc(length + 1); // allocate it with space for 0x00
            memset(buf, 0x00, length + 1); // wipe it up

            // move until we find the ;
            while ((t < length) && (incipit[t] != ';')) {

                // we should not find any close paren before the ';' !
                // FIXME find a graceful way to exit signaling this to user
                if (incipit[t] == ')') {
                    LogDebug("You have a) before the ; in a tuplet!");
                    free(buf);
                    return i - index;
                }

                t++;
            }

            // t + 1 should point to the number
            t++; // move one char to the number
            while (((t + t2) < length) && (incipit[t + t2] != ')')) {

                // If we have extraneous chars, exit here
                if (!isdigit(incipit[t + t2])) {
                    LogDebug("You have a non-number in a tuplet number");
                    free(buf);
                    return i - index;
                }

                // copy the number char-by-char
                buf[t2] = incipit[t + t2];
                t2++;
            }

            tuplet_val = atoi(buf);
            free(buf); // dispose of the buffer
        }
        else { // it is a triplet
            // don't care to parse all the stuff
            tuplet_val = 3;
        }

        // this is the first note, the total number of notes = tuplet_val
        note->tuplet_notes = tuplet_val;
        // but also the note counter
        note->tuplet_note = tuplet_val;
    }
    else {
        if (note->tuplet_notes > 0) {
            LogWarning("Fermata within a tuplet. Won't be handled correctly");
        }
        note->fermata = true;
    }

    return i - index;
}

//////////////////////////////
//
// getTupletFermataEnd --
//
// this can be deleted in the future?
int PaeInput::getTupletFermataEnd(const char *incipit, pae::Note *note, int index)
{
    int i = index;
    // int length = strlen(incipit);

    // TODO: fermatas inside tuplets won't be currently handled correctly
    note->fermata = false;

    return i - index;
}

//////////////////////////////
//
// getGraceNote --
//

int PaeInput::getGraceNote(const char *incipit, pae::Note *note, int index)
{
    int i = index;
    int length = (int)strlen(incipit);

    // acciaccatura
    if (incipit[i] == 'g') {
        note->acciaccatura = true;
    }

    // appoggiatura
    else if (incipit[i] == 'q') {
        note->appoggiatura = 1;
        if ((i + 1 < length) && (incipit[i + 1] == 'q')) {
            i++;
            int r = i;
            while ((r < length) && (incipit[r] != 'r')) {
                if ((incipit[r] - 'A' >= 0) && (incipit[r] - 'A' < 7)) {
                    note->appoggiatura++;
                }
                r++;
            }
        }
    }
    return i - index;
}

//////////////////////////////
//
// getPitch --
//

data_PITCHNAME PaeInput::getPitch(char c_note)
{
    data_PITCHNAME pitch = PITCHNAME_c;

    switch (c_note) {
        case 'A': pitch = PITCHNAME_a; break;
        case 'B': pitch = PITCHNAME_b; break;
        case 'C': pitch = PITCHNAME_c; break;
        case 'D': pitch = PITCHNAME_d; break;
        case 'E': pitch = PITCHNAME_e; break;
        case 'F': pitch = PITCHNAME_f; break;
        case 'G': pitch = PITCHNAME_g; break;
        case '-': pitch = PITCHNAME_NONE; break;
        default: break;
    }
    return pitch;
}

//////////////////////////////
//
// getTimeInfo -- read the key signature.
//

int PaeInput::getTimeInfo(const char *incipit, MeterSig *meter, int index)
{
    int i = index;
    int length = (int)strlen(incipit);

    if (!isdigit(incipit[i]) && (incipit[i] != 'c') && (incipit[i] != 'o')) {
        return 0;
    }

    // find the end of time signature
    i++; // the time signature length is a least 1
    while (i < length) {
        if (!isdigit(incipit[i]) && (incipit[i] != '/') && (incipit[i] != '.')) {
            break;
        }
        i++;
    }

    // use a substring for the time signature
    char timesig_str[1024];
    memset(timesig_str, 0, 1024);
    // strncpy not always put the \0 in the end!
    strncpy(timesig_str, incipit + index, i - index);

    std::ostringstream sout;

    // regex_match matches to the entire input string (regex_search does
    // partial matches.  In this case cmatch is used to store the submatches
    // (enclosed in parentheses) for later reference.  Use std::smatch when
    // dealing with strings, or std::wmatch with wstrings.
    std::cmatch matches;
    if (regex_match(timesig_str, matches, std::regex("(\\d+)/(\\d+)"))) {
        meter->SetCount(std::stoi(matches[1]));
        meter->SetUnit(std::stoi(matches[2]));
    }
    else if (regex_match(timesig_str, matches, std::regex("\\d+"))) {
        meter->SetCount(std::stoi(timesig_str));
    }
    else if (strcmp(timesig_str, "c") == 0) {
        // C
        meter->SetSym(METERSIGN_common);
    }
    else if (strcmp(timesig_str, "c/") == 0) {
        // C|
        meter->SetSym(METERSIGN_cut);
    }
    else if (strcmp(timesig_str, "c3") == 0) {
        // C3
        meter->SetSym(METERSIGN_common);
        meter->SetCount(3);
    }
    else if (strcmp(timesig_str, "c3/2") == 0) {
        // C3/2
        meter->SetSym(METERSIGN_common); // ??
        meter->SetCount(3);
        meter->SetUnit(2);
    }
    else {
        LogWarning("Unknown time signature: %s", timesig_str);
    }

    return i - index;
}

//////////////////////////////
//
// getClefInfo -- read the key signature.
//

int PaeInput::getClefInfo(const char *incipit, Clef *mclef, int index)
{
    // a clef is maximum 3 character length
    // go through the 3 character and retrieve the letter (clef) and the line
    // mensural clef (with + in between) currently ignored
    // clef with octava correct?
    int length = (int)strlen(incipit);
    int i = 0;
    char clef = 'G';
    char line = '2';
    while ((index < length) && (i < 3)) {
        if (i == 0) {
            clef = incipit[index];
        }
        else if (i == 2) {
            line = incipit[index];
        }
        i++;
        index++;
    }

    if (clef == 'C' || clef == 'c') {
        mclef->SetShape(CLEFSHAPE_C);
        mclef->SetLine(line - 48);
    }
    else if (clef == 'G') {
        mclef->SetShape(CLEFSHAPE_G);
        mclef->SetLine(line - 48);
    }
    else if (clef == 'g') {
        mclef->SetShape(CLEFSHAPE_G);
        mclef->SetLine(line - 48);
        mclef->SetDis(OCTAVE_DIS_8);
        mclef->SetDisPlace(PLACE_below);
    }
    else if (clef == 'F' || clef == 'f') {
        mclef->SetShape(CLEFSHAPE_F);
        mclef->SetLine(line - 48);
    }
    else {
        // what the...
        LogDebug("Clef %c is Undefined", clef);
    }

    // measure->clef = mclef;

    return i;
}

//////////////////////////////
//
// getWholeRest -- read the getWholeRest.
//

int PaeInput::getWholeRest(const char *incipit, int *wholerest, int index)
{
    int length = (int)strlen(incipit);
    int i = index;

    *wholerest = 1;
    if ((i + 1 < length) && isdigit(incipit[i + 1])) {
        sscanf(&(incipit[i + 1]), "%d", wholerest);
        char buf[10];
        memset(buf, 0, 10);
        sprintf(buf, "%d", *wholerest);
        i += strlen(buf);
    }
    return i - index;
}

/**********************************
 *
 * getBarLine -- read the barLine.
 * Translation from PAE to verovio representaion:
 *
 BARRENDITION_single     /
 BARRENDITION_end        does not exist
 BARRENDITION_rptboth    ://:
 BARRENDITION_rptend     ://
 BARRENDITION_rptstart   //:
 BARRENDITION_dbl        //
 */

int PaeInput::getBarLine(const char *incipit, data_BARRENDITION *output, int index)
{

    bool is_barline_rptboth = false;
    bool is_barline_rptend = false;
    bool is_barline_rptstart = false;
    bool is_barline_dbl = false;

    if (strncmp(incipit + index, "://:", 4) == 0) {
        is_barline_rptboth = true;
    }

    if (strncmp(incipit + index, "://", 3) == 0) {
        is_barline_rptend = true;
    }

    if (strncmp(incipit + index, "//:", 3) == 0) {
        is_barline_rptstart = true;
    }

    if (strncmp(incipit + index, "//", 2) == 0) {
        is_barline_dbl = true;
    }

    int i = 0; // number of characters
    if (is_barline_rptboth) {
        *output = BARRENDITION_rptboth;
        i = 3;
    }
    else if (is_barline_rptstart) {
        *output = BARRENDITION_rptstart;
        i = 2;
    }
    else if (is_barline_rptend) {
        *output = BARRENDITION_rptend;
        i = 2;
    }
    else if (is_barline_dbl) {
        *output = BARRENDITION_dbl;
        i = 1;
    }
    else {
        *output = BARRENDITION_single;
        i = 0;
    }
    return i;
}

//////////////////////////////
//
// getAbbreviation -- read abbreviation
//

int PaeInput::getAbbreviation(const char *incipit, pae::Measure *measure, int index)
{
    int length = (int)strlen(incipit);
    int i = index;
    int j;

    if (measure->abbreviation_offset == -1) { // start
        measure->abbreviation_offset = (int)measure->notes.size();
    }
    else { //
        int abbreviation_stop = (int)measure->notes.size();
        while ((i + 1 < length) && (incipit[i + 1] == 'f')) {
            i++;
            for (j = measure->abbreviation_offset; j < abbreviation_stop; j++) {
                measure->notes.push_back(measure->notes[j]);
            }
        }
        measure->abbreviation_offset = -1;
    }

    return i - index;
}

//////////////////////////////
//
// getKeyInfo -- read the key signature.
//

int PaeInput::getKeyInfo(const char *incipit, KeySig *key, int index)
{
    int alt_nr = 0;

    // at the key information line, extract data
    int length = (int)strlen(incipit);
    int i = index;
    bool end_of_keysig = false;
    while ((i < length) && (!end_of_keysig)) {
        switch (incipit[i]) {
            case 'b': key->SetAlterationType(ACCIDENTAL_EXPLICIT_f); break;
            case 'x': key->SetAlterationType(ACCIDENTAL_EXPLICIT_s); break;
            case 'n': key->SetAlterationType(ACCIDENTAL_EXPLICIT_n); break;
            case '[': break;
            case 'F':
            case 'C':
            case 'G':
            case 'D':
            case 'A':
            case 'E':
            case 'B': alt_nr++; break;
            default: end_of_keysig = true; break;
        }
        if (!end_of_keysig) i++;
    }

    if (key->GetAlterationType() != ACCIDENTAL_EXPLICIT_n) {
        key->SetAlterationNumber(alt_nr);
    }

    key->ConvertToMei();

    return i - index;
}

//////////////////////////////
//
// getNote --
//

int PaeInput::getNote(const char *incipit, pae::Note *note, pae::Measure *measure, int index)
{
    int oct;
    int i = index;
    int app;
    int tuplet_num;

    if (note->acciaccatura) {
        // acciaccaturas are always eights regardless
        // and have no dots
        note->duration = DURATION_8;
    }
    else {
        if (measure->durations.size() == 0) {
            note->duration = DURATION_4;
            note->dots = 0;
            LogWarning("Got a note before a duration was specified");
        }
        else {
            note->duration = measure->durations[measure->durations_offset];
            note->dots = measure->dots[measure->durations_offset];
        }
    }
    note->pitch = getPitch(incipit[i]);

    // lookout, hack. If a rest (PITCHNAME_NONE val) then create rest object.
    // it will be added instead of the note
    if (note->pitch == PITCHNAME_NONE) {
        note->rest = true;
    }

    // trills
    if (regex_search(incipit + i + 1, std::regex("^[^A-G]*t"))) {
        note->trill = true;
    }

    // tie
    if (regex_search(incipit + i + 1, std::regex("^[A-G]*\\+"))) {
        // reset 1 for first note, >1 for next ones is incremented under
        if (note->tie == 0) note->tie = 1;
    }

    // chord
    if (regex_search(incipit + i + 1, std::regex("^[^A-G]*\\^"))) {
        note->chord = true;
    }

    oct = note->octave;
    measure->notes.push_back(*note);

    app = note->appoggiatura;
    tuplet_num = note->tuplet_note;

    // Reset note to defaults
    note->clear();

    // write back the values we need to save
    note->octave = oct; // save octave

    // tuplets. Decrease the current number if we are in a tuplet
    // i.e. tuplet_num > 0
    // al the other values just need to be in the first note
    if (tuplet_num > 0) {
        note->tuplet_note = --tuplet_num;
    }

    // grace notes
    note->acciaccatura = false;
    if (app > 0) {
        note->appoggiatura = --app;
    }
    // durations
    if (measure->durations.size() > 0) {
        measure->durations_offset++;
        if (measure->durations_offset >= (int)measure->durations.size()) {
            measure->durations_offset = 0;
        }
    }

    note->fermata = false; // only one note per fermata;
    note->trill = false;

    return i - index;
}

//////////////////////////////
//
// convertMeasure --
//

void PaeInput::convertMeasure(pae::Measure *measure)
{
    if (measure->clef != NULL) {
        m_layer->AddChild(measure->clef);
    }

    if (measure->wholerest > 0) {
        MultiRest *mr = new MultiRest();
        mr->SetNum(measure->wholerest);
        m_layer->AddChild(mr);
    }

    m_nested_objects.clear();

    for (unsigned int i = 0; i < measure->notes.size(); i++) {
        pae::Note *note = &measure->notes[i];
        parseNote(note);
    }

    // Set barLine
    m_measure->SetRight(measure->barLine);
}

void PaeInput::parseNote(pae::Note *note)
{

    LayerElement *element;

    if (note->rest) {
        Rest *rest = new Rest();

        rest->SetDots(note->dots);
        rest->SetDur(note->duration);

        if (note->fermata) {
            rest->SetFermata(PLACE_above); // always above for now
        }

        element = rest;
    }
    else {
        Note *mnote = new Note();

        mnote->SetPname(note->pitch);
        mnote->SetOct(note->octave);
        mnote->SetAccid(note->accidental);

        mnote->SetDots(note->dots);
        mnote->SetDur(note->duration);

        // pseudo chant notation with 7. in PAE - make quater notes without stem
        if ((mnote->GetDur() == DURATION_128) && (mnote->GetDots() == 1)) {
            mnote->SetDur(DURATION_4);
            mnote->SetDots(0);
            mnote->SetStemLen(0);
        }

        if (note->fermata) {
            mnote->SetFermata(PLACE_above); // always above for now
        }

        if (note->trill == true) {
            mnote->m_embellishment = EMB_TRILL;
        }

        if (m_last_tied_note != NULL) {
            mnote->SetTie(TIE_t);
            m_last_tied_note = NULL;
        }

        if (note->tie) {
            if (mnote->GetTie() == TIE_t)
                mnote->SetTie(TIE_m);
            else
                mnote->SetTie(TIE_i);
            m_last_tied_note = mnote;
        }

        element = mnote;
    }

    // Does this note have a clef change? push it before everything else
    if (note->clef) {
        addLayerElement(note->clef);
    }

    // Same thing for time changes
    // You can find this sometimes
    if (note->meter) {
        addLayerElement(note->meter);
    }

    // Handle key change. Evil if done in a beam
    if (note->key) {
        addLayerElement(note->key);
    }

    // Acciaccaturas are similar but do not get beamed (do they)
    // this case is simpler. NOTE a note can not be acciacctura AND appoggiatura
    // Acciaccatura rests do not exist
    if (note->acciaccatura && (element->Is() == NOTE)) {
        Note *mnote = dynamic_cast<Note *>(element);
        assert(mnote);
        mnote->SetDur(DURATION_8);
        mnote->SetGrace(GRACE_acc);
        mnote->SetStemDir(STEMDIRECTION_up);
    }

    if ((note->appoggiatura > 0) && (element->Is() == NOTE)) {
        Note *mnote = dynamic_cast<Note *>(element);
        assert(mnote);
        mnote->SetGrace(GRACE_unacc);
        mnote->SetStemDir(STEMDIRECTION_up);
    }

    if (note->beam == BEAM_INITIAL) {
        pushContainer(new Beam());
    }

    // we have a tuplet, the tuplet_note is > 0
    // which means we are counting a tuplet
    if (note->tuplet_note > 0 && note->tuplet_notes == note->tuplet_note) { // first elem in tuplet
        Tuplet *newTuplet = new Tuplet();
        newTuplet->SetNum(note->tuplet_notes);
        newTuplet->SetNumbase(note->tuplet_notes);
        pushContainer(newTuplet);
    }

    // note in a chord
    if (note->chord) {
        Note *mnote = dynamic_cast<Note *>(element);
        assert(mnote);
        // first note?
        if (!m_is_in_chord) {
            Chord *chord = new Chord();
            chord->SetDots(mnote->GetDots());
            chord->SetDur(mnote->GetDur());
            pushContainer(chord);
            m_is_in_chord = true;
        }
        mnote->SetDots(0);
        mnote->SetDur(DURATION_NONE);
    }

    // Add the note to the current container
    addLayerElement(element);

    // the last note counts always '1'
    // insert the tuplet elem
    // and reset the tuplet counter
    if (note->tuplet_note == 1) {
        popContainer();
    }

    if (note->beam == BEAM_TERMINAL) {
        popContainer();
    }

    // last note of a chord
    if (!note->chord && m_is_in_chord) {
        Note *mnote = dynamic_cast<Note *>(element);
        assert(mnote);
        mnote->SetDots(0);
        mnote->SetDur(DURATION_NONE);
        popContainer();
        m_is_in_chord = false;
    }
}

void PaeInput::pushContainer(LayerElement *container)
{
    addLayerElement(container);
    m_nested_objects.push_back(container);
}

void PaeInput::popContainer()
{
    // assert(m_nested_objects.size() > 0);
    if (m_nested_objects.size() == 0) {
        LogError("PaeInput::popContainer: tried to pop an object from empty stack. "
                 "Cross-measure objects (tuplets, beams) are not supported.");
    }
    else {
        m_nested_objects.pop_back();
    }
}

void PaeInput::addLayerElement(LayerElement *element)
{
    if (m_nested_objects.size() > 0) {
        m_nested_objects.back()->AddChild(element);
    }
    else {
        m_layer->AddChild(element);
    }
}

//////////////////////////////
//
// getAtRecordKeyValue --
//   Formats: @key: value
//            @key:value
//            @key :value
//            @ key :value
//            @ key : value
//   only one per line
//

void PaeInput::getAtRecordKeyValue(char *key, char *value, const char *input)
{

#define SKIPSPACE                                                                                                      \
    while ((index < length) && isspace(input[index])) {                                                                \
        index++;                                                                                                       \
    }

    char MARKER = '@';
    char SEPARATOR = ':';
    char EMPTY = '\0';

    int length = (int)strlen(input);
    int count = 0;

    // zero out strings
    memset(key, EMPTY, MAX_DATA_LEN);
    memset(value, EMPTY, MAX_DATA_LEN);

    if (length == 0) {
        return;
    }

    char ch;
    int index = 0;

    // find starting @ symbol (ignoring any starting space)
    SKIPSPACE
    if (input[index] != MARKER) {
        // invalid record format since it does not start with @
        return;
    }
    index++;
    SKIPSPACE

    // start storing the key value:
    while ((index < length) && (input[index] != SEPARATOR)) {
        if (isspace(input[index])) {
            continue;
        }
        ch = input[index];

        // Should never happen
        if (count >= MAX_DATA_LEN) return;

        key[count] = ch;
        count++;
        index++;
    }
    // check to see if valid format: (:) must be the current character
    if (input[index] != SEPARATOR) {
        key[0] = EMPTY;
        return;
    }
    index++;
    SKIPSPACE

    // Also should never happen
    if (strlen(&input[index]) > MAX_DATA_LEN) return;

    strcpy(value, &input[index]);

    // Truncate string to first space
    size_t i;
    for (i = strlen(value) - 2; i > 0; i--) {
        if (isspace(value[i])) {
            value[i] = EMPTY;
            continue;
        }
        break;
    }
}

#endif // NO_PAE_SUPPORT

} // namespace vrv
