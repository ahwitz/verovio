/////////////////////////////////////////////////////////////////////////////
// Name:        musiopae.h
// Author:      Rodolfo Zitellini
// Created:     2012
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#ifndef __VRV_IOPAE_H__
#define __VRV_IOPAE_H__

#include <string>
#include <vector>

//----------------------------------------------------------------------------

#include "io.h"
#include "vrvdef.h"

namespace vrv {

class Beam;
class Clef;
class Layer;
class LayerElement;
class Measure;
class MeterSig;
class Note;
class Staff;
class Tie;
class Tuplet;
class KeySig;
class Barline;


class NoteObject {
public:
    NoteObject(const NoteObject &old) { // for STL vector
        //mnote = old.mnote;
        //mrest = old.mrest;         
        tie = old.tie;
        acciaccatura = old.acciaccatura;
        appoggiatura = old.appoggiatura;
        fermata = old.fermata;
        trill = old.trill;
        
        octave = old.octave;
        beam = old.beam;
        pitch = old.pitch;
        duration = old.duration;
        accidental = old.accidental;
        dots = old.dots;
        rest = old.rest;
        
        clef = old.clef;
        meter = old.meter;
        key = old.key;

        tuplet_notes = old.tuplet_notes;
        tuplet_note = old.tuplet_note;
    }
    NoteObject(void) { clear(); };
    void   clear(void) {
        appoggiatura = 0;
        acciaccatura = fermata = trill = false;
        tie = 0;
        
        octave = 4;
        beam = 0;
        pitch = PITCHNAME_NONE;
        duration = DURATION_NONE;
        accidental = ACCIDENTAL_EXPLICIT_NONE;
        dots = 0;
        rest = false;
        
        tuplet_notes = 0;
        tuplet_note = 0;
        
        clef = NULL;
        meter = NULL;
        key = NULL;
    };
    
    NoteObject& operator=(const NoteObject& d){ // for STL vector
        //mnote = d.mnote;
        //mrest = d.mrest;         
        tie = d.tie;
        acciaccatura = d.acciaccatura;
        appoggiatura = d.appoggiatura;
        fermata = d.fermata;
        trill = d.trill;
        
        octave = d.octave;
        beam = d.beam;
        pitch = d.pitch;
        duration = d.duration;
        accidental = d.accidental;
        dots = d.dots;
        rest = d.rest;
        
        clef = d.clef;
        meter = d.meter;
        key = d.key;
        
        tuplet_notes = d.tuplet_notes;
        tuplet_note = d.tuplet_note;
        
        return *this;
    }
    
    //Note *mnote;
    //Rest *mrest; // this is not too nice

    // tuplet stuff
    int tuplet_notes; // quantity of notes in the tuplet
    int tuplet_note; // indicates this note is the nth in the tuplet
    
    int    tie;
    bool   acciaccatura;
    int    appoggiatura;
    bool   fermata;
    bool   trill;
    
    char octave;
    unsigned char beam;
    data_PITCHNAME pitch;
    data_DURATION duration;
    data_ACCIDENTAL_EXPLICIT accidental;
    unsigned int dots;
    bool rest;
    
    Clef *clef;
    MeterSig *meter;
    KeySig *key;
    
};


class MeasureObject {
public:
    
    MeasureObject(const MeasureObject& d){ // for STL vector
        clef = d.clef;
        meter = d.meter;
        notes = d.notes;
        
        key = d.key;
        
        durations = d.durations;
        dots = d.dots; 
        durations_offset = d.durations_offset;
        barLine = d.barLine;
        abbreviation_offset = d.abbreviation_offset;  
        wholerest = d.wholerest;
    } 
    MeasureObject(void) { clear(); };
    
    MeasureObject& operator=(const MeasureObject& d){ // for STL vector
        clef = d.clef;
        meter = d.meter;
        notes = d.notes;
        
        key = d.key;
        
        durations = d.durations;
        dots = d.dots; 
        durations_offset = d.durations_offset;
        barLine = d.barLine;
        abbreviation_offset = d.abbreviation_offset;  
        wholerest = d.wholerest;   
        return *this;
    } 
     
    void   clear(void) {
        durations.clear();
        dots.clear();
        notes.clear();
        durations_offset = DURATION_long;
        reset();
    };
    void   reset(void) {
        clef = NULL;
        meter = NULL;
        key = NULL;
        notes.clear();
        barLine = BARRENDITION_NONE;
        wholerest = 0; 
        abbreviation_offset = -1;
    };
    Clef *clef;
    MeterSig *meter;
    KeySig *key;
    
    std::vector<NoteObject> notes;
    
    std::vector<data_DURATION> durations;
    std::vector<int> dots; // use the same offset as durations, they are used in parallel
    char durations_offset;
    data_BARRENDITION    barLine;
    int    abbreviation_offset;  
    int    wholerest;   // number of whole rests to process
};


//////////////////////////////////////////////////////////////////////////









//----------------------------------------------------------------------------
// PaeInput
//----------------------------------------------------------------------------


class PaeInput: public FileInputStream
{
public:
    // constructors and destructors
    PaeInput( Doc *doc, std::string filename );
    virtual ~PaeInput();
    
    virtual bool ImportFile( );
    virtual bool ImportString(std::string pae);

#ifndef NO_PAE_SUPPORT

private:
    // function declarations:
    
     void      parsePlainAndEasy( std::istream &infile );
     
     // parsing functions
     int       getKeyInfo          (const char* incipit, KeySig *key, int index = 0);
     int       getTimeInfo         (const char* incipit, MeterSig *meter, int index = 0);
     int       getClefInfo         (const char* incipit, Clef *mus_clef, int index = 0 );
     int       getBarline          (const char *incipit, data_BARRENDITION *output, int index );
     int       getAccidental       (const char* incipit, data_ACCIDENTAL_EXPLICIT *accident, int index = 0);
     int       getOctave           (const char* incipit, char *octave, int index = 0 );
     int       getDurations        (const char* incipit, MeasureObject *measure, int index = 0);
     int       getDuration         (const char* incipit, data_DURATION *duration, int *dot, int index );
     int       getTupletFermata    (const char* incipit, NoteObject *note, int index = 0);
     int       getTupletFermataEnd (const char* incipit, NoteObject *note, int index = 0);
     int       getGraceNote        (const char* incipit, NoteObject *note, int index = 0);
     int       getWholeRest        (const char* incipit, int *wholerest, int index );
     int       getAbbreviation     (const char* incipit, MeasureObject *measure, int index = 0 ); 
     int       getNote             (const char* incipit, NoteObject *note, MeasureObject *measure, int index = 0 );
     
     data_PITCHNAME       getPitch            (char c_note );
     
     // output functions
     void      addLayerElement     (LayerElement *element);
     void      parseNote           (NoteObject note);
     void      popContainer        ();
     void      convertMeasure        (MeasureObject *measure);
     void      pushContainer       (LayerElement *container);


     // input functions
     void      getAtRecordKeyValue (char *key, char* value, const char* input);

#endif // NO_PAE_SUPPORT
    
public:
    
private:
    std::string m_filename;
	Staff *m_staff;
    Measure *m_measure;
	Layer *m_layer;
    Note *m_last_tied_note;

    std::vector<LayerElement *> m_nested_objects;
};

} // namespace vrv

#endif
