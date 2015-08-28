/////////////////////////////////////////////////////////////////////////////
// Name:        note.cpp
// Author:      Laurent Pugin
// Created:     2011
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#include "note.h"

//----------------------------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------------------------

#include "editorial.h"
#include "slur.h"
#include "tie.h"
#include "verse.h"
#include "vrv.h"

namespace vrv {

//----------------------------------------------------------------------------
// Note
//----------------------------------------------------------------------------

Note::Note():
	LayerElement("note-"), DurationInterface(), PitchInterface(),
    AttColoration(),
    AttGraced(),
    AttNoteLogMensural(),
    AttStemmed(),
    AttTiepresent()
{
    RegisterAttClass(ATT_COLORATION);
    RegisterAttClass(ATT_GRACED);
    RegisterAttClass(ATT_NOTELOGMENSURAL);
    RegisterAttClass(ATT_STEMMED);
    RegisterAttClass(ATT_TIEPRESENT);
    
    RegisterInterface( DurationInterface::GetAttClasses(), DurationInterface::IsInterface() );
    RegisterInterface( PitchInterface::GetAttClasses(), PitchInterface::IsInterface() );
    
    m_drawingTieAttr = NULL;
    m_drawingAccid = NULL;
    m_isDrawingAccidAttr = false;
    
    Reset();
}


Note::~Note()
{
    // This deletes the Tie, Slur, and Accid objects if necessary
    if (m_drawingTieAttr) {
        delete m_drawingTieAttr;
    }
    // We delete it only if it is an attribute - otherwise we do not own it
    if (m_drawingAccid && m_isDrawingAccidAttr) {
        delete m_drawingAccid;
    }
    
}
    
void Note::Reset()
{
    LayerElement::Reset();
    DurationInterface::Reset();
    PitchInterface::Reset();
    
    ResetColoration();
    ResetGraced();
    ResetNoteLogMensural();
    ResetStemmed();
    ResetTiepresent();
    
    // TO BE REMOVED
    m_embellishment = EMB_NONE;
    // tie pointers
    ResetDrawingTieAttr();
    // accid pointer
    ResetDrawingAccid();
    
    m_drawingStemDir = STEMDIRECTION_NONE;
    d_stemLen = 0;
    m_clusterPosition = 0;
    m_cluster = NULL;
    m_graceAlignment = NULL;
}

void Note::AddLayerElement(vrv::LayerElement *element)
{
    assert(dynamic_cast<Accid*>(element)
        || dynamic_cast<Verse*>(element)
        || dynamic_cast<EditorialElement*>(element) );
    element->SetParent( this );
    m_children.push_back(element);
    Modify();
}

Alignment* Note::GetGraceAlignment(  )
{
    assert(m_graceAlignment);
    return m_graceAlignment;
}

void Note::SetGraceAlignment( Alignment *graceAlignment )
{
    assert(!m_graceAlignment && graceAlignment);
    m_graceAlignment = graceAlignment;
}
    
void Note::SetDrawingTieAttr(  )
{
    assert(!this->m_drawingTieAttr);
    if ( m_drawingTieAttr ) return;
    m_drawingTieAttr = new Tie();
    m_drawingTieAttr->SetStart( this );
}

void Note::ResetDrawingTieAttr( )
{
    if ( m_drawingTieAttr ) {
        delete m_drawingTieAttr;
        m_drawingTieAttr = NULL;
    }
}
  
void Note::ResetDrawingAccid( )
{
    if ( m_drawingAccid ) {
        // We delete it only if it is an attribute - otherwise we do not own it
        if (m_isDrawingAccidAttr) delete m_drawingAccid;
        m_drawingAccid = NULL;
        m_isDrawingAccidAttr = false;
    }
    // we should never have no m_drawingAccid but have the attr flag to true
    assert( !m_isDrawingAccidAttr );
}
    
Chord* Note::IsChordTone()
{
    return dynamic_cast<Chord*>(this->GetFirstParent( CHORD, MAX_CHORD_DEPTH) );
}
    
int Note::GetDrawingDur( )
{
    Chord* chordParent = dynamic_cast<Chord*>(this->GetFirstParent( CHORD, MAX_CHORD_DEPTH));
    if( chordParent ) {
        return chordParent->GetActualDur();
    }
    else {
        return GetActualDur();
    }
}
    
bool Note::IsClusterExtreme()
{
    ChordCluster* cluster = this->m_cluster;
    if (this == cluster->at(0)) return true;
    if (this == cluster->at(cluster->size() - 1)) return true;
    else return false;
}
    
data_STEMDIRECTION Note::CalcDrawingStemDir()
{
    Chord* chordParent = dynamic_cast<Chord*>(this->GetFirstParent( CHORD, MAX_CHORD_DEPTH));
    Beam* beamParent = dynamic_cast<Beam*>(this->GetFirstParent( BEAM, MAX_BEAM_DEPTH));
    if( chordParent ) {
        return chordParent->GetDrawingStemDir();
    }
    else if( beamParent ) {
        return beamParent->GetDrawingStemDir();
    }
    else {
        return this->GetStemDir();
    }
}
    
//----------------------------------------------------------------------------
// Functors methods
//----------------------------------------------------------------------------

int Note::PrepareTieAttr( ArrayPtrVoid *params )
{
    // param 0: std::vector<Note*>* that holds the current notes with open ties
    // param 1: Chord** currentChord for the current chord if in a chord
    std::vector<Note*> *currentNotes = static_cast<std::vector<Note*>*>((*params)[0]);
    Chord **currentChord = static_cast<Chord**>((*params)[1]);
    
    AttTiepresent *check = this;
    if ((*currentChord)) {
        check = (*currentChord);
    }
    assert(check);
    
    std::vector<Note*>::iterator iter = currentNotes->begin();
    while ( iter != currentNotes->end()) {
        // same octave and same pitch - this is the one!
        if ((this->GetOct()==(*iter)->GetOct()) && (this->GetPname()==(*iter)->GetPname())) {
            // right flag
            if ((check->GetTie()==TIE_m) || (check->GetTie()==TIE_t)) {
                assert( (*iter)->GetDrawingTieAttr() );
                (*iter)->GetDrawingTieAttr()->SetEnd(this);
            }
            else {
                LogWarning("Expected @tie median or terminal in note '%s', skipping it", this->GetUuid().c_str());
                (*iter)->ResetDrawingTieAttr();
            }
            iter = currentNotes->erase( iter );
            // we are done for this note
            break;
        }
        iter++;
    }

    if ((check->GetTie()==TIE_m) || (check->GetTie()==TIE_i)) {
        this->SetDrawingTieAttr();
        currentNotes->push_back(this);
    }
    
    return FUNCTOR_CONTINUE;
}
    

int Note::FillStaffCurrentTimeSpanning( ArrayPtrVoid *params )
{
    // Pass it to the pseudo functor of the interface
    if (this->m_drawingTieAttr) {
        return this->m_drawingTieAttr->FillStaffCurrentTimeSpanning(params);
    }
    return FUNCTOR_CONTINUE;
}
    
int Note::PrepareLyrics( ArrayPtrVoid *params )
{
    // param 0: the current Syl (unused)
    // param 1: the last Note
    // param 2: the last but one Note
    Note **lastNote = static_cast<Note**>((*params)[1]);
    Note **lastButOneNote = static_cast<Note**>((*params)[2]);
    
    (*lastButOneNote) = (*lastNote);
    (*lastNote) = this;
    
    return FUNCTOR_CONTINUE;
}
    
int Note::PreparePointersByLayer( ArrayPtrVoid *params )
{
    // param 0: the current Note
    Note **currentNote = static_cast<Note**>((*params)[0]);
    
    this->ResetDrawingAccid();
    if (this->GetAccid() != ACCIDENTAL_EXPLICIT_NONE) {
        this->m_isDrawingAccidAttr = true;
        this->m_drawingAccid = new Accid();
        this->m_drawingAccid->SetOloc(this->GetOct());
        this->m_drawingAccid->SetPloc(this->GetPname());
        this->m_drawingAccid->SetAccid(this->GetAccid());
        // We need to set the drawing cue size since there will be no access to the note
        this->m_drawingAccid->m_drawingCueSize = this->HasGrace();
    }
    
    (*currentNote) = this;
    
    return FUNCTOR_CONTINUE;
}
    
int Note::ResetDarwing( ArrayPtrVoid *params )
{
    this->ResetDrawingTieAttr();
    return FUNCTOR_CONTINUE;
};

} // namespace vrv
