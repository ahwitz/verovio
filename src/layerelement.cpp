/////////////////////////////////////////////////////////////////////////////
// Name:        layerelement.cpp
// Author:      Laurent Pugin
// Created:     2011
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////



//----------------------------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------------------------

#include "accid.h"
#include "aligner.h"
#include "att_comparison.h"
#include "barline.h"
#include "beam.h"
#include "chord.h"
#include "clef.h"
#include "custos.h"
#include "doc.h"
#include "dot.h"
#include "keysig.h"
#include "layer.h"
#include "measure.h"
#include "mensur.h"
#include "metersig.h"
#include "mrest.h"
#include "multirest.h"
#include "note.h"
#include "rest.h"
#include "space.h"
#include "staff.h"
#include "syl.h"
#include "tie.h"
#include "timeinterface.h"
#include "tuplet.h"
#include "verse.h"
#include "view.h"
#include "vrv.h"

namespace vrv {

//----------------------------------------------------------------------------
// LayerElement
//----------------------------------------------------------------------------

LayerElement::LayerElement():
    DocObject("le-")
{
    Reset();
}

LayerElement::LayerElement(std::string classid):
	DocObject(classid)
{
    Reset();
}

void LayerElement::Reset()
{
    DocObject::Reset();
    
    m_xAbs = VRV_UNSET;
    m_drawingX = 0;
    m_drawingY = 0;
    
    m_isScoreOrStaffDefAttr = false;
    m_alignment = NULL;
    m_drawingStemDir = STEMDIRECTION_NONE;
    m_beamElementCoord = NULL;
}
    
LayerElement::~LayerElement()
{
    
}

LayerElement& LayerElement::operator=( const LayerElement& element )
{
	if ( this != &element ) // not self assignement
	{
        // pointers have to be NULL
        m_parent = NULL;
        m_alignment = NULL;
	}
	return *this;
}
    
void LayerElement::ResetHorizontalAlignment()
{
    m_drawingX = 0;
    m_alignment = NULL;
    if (this->Is() == NOTE) {
        Note *note = dynamic_cast<Note*>(this);
        assert( note );
        note->ResetGraceAlignment();
    }
}

bool LayerElement::IsGraceNote()
{
    Note *note = dynamic_cast<Note*>(this);
    return (note && note->HasGrace());
}

bool LayerElement::IsCueSize()
{
    if ( this->Is() == NOTE ) {
        Note *note = dynamic_cast<Note*>(this);
        assert( note );
        return ( note->HasGrace() );
    }
    Note *note = dynamic_cast<Note*>(this->GetFirstParent( NOTE, MAX_ACCID_DEPTH ) );
    return ( note && ( note->HasGrace() ) );
}
    
void LayerElement::AdjustPname( int *pname, int *oct )
{
	if ((*pname) < PITCHNAME_c)
	{
		if ((*oct) > 0)
			(*oct)-- ;
        (*pname) = PITCHNAME_b;
        
	}
	else if ((*pname) > PITCHNAME_b)
	{
		if ((*oct) < 7)
			(*oct)++;
        (*pname) = PITCHNAME_c;
	}
}

double LayerElement::GetAlignmentDuration( Mensur *mensur, MeterSig *meterSig, bool notGraceOnly )
{
    if ( this->IsGraceNote() && notGraceOnly ) {
        return 0.0;
    }
    
    if ( this->HasInterface(INTERFACE_DURATION) ) {
        int num = 1;
        int numbase = 1;
        Tuplet *tuplet = dynamic_cast<Tuplet*>( this->GetFirstParent( TUPLET, MAX_TUPLET_DEPTH ) );
        if ( tuplet ) {
            num = tuplet->GetNum();
            numbase = tuplet->GetNumbase();
        }
        DurationInterface *duration = dynamic_cast<DurationInterface*>(this);
        assert( duration );
        if (duration->IsMensural()) return duration->GetAlignmentMensuralDuration( num, numbase, mensur );
        else return duration->GetAlignmentDuration( num, numbase );
    }
    else {
        return 0.0;
    }
}

int LayerElement::GetXRel()
{
    if (m_alignment) {
        return m_alignment->GetXRel();
    }
    return 0;
}
    
//----------------------------------------------------------------------------
// LayerElement functors methods
//----------------------------------------------------------------------------

int LayerElement::AlignHorizontally( ArrayPtrVoid *params )
{
    // param 0: the measureAligner
    // param 1: the time
    // param 2: the current Mensur
    // param 3: the current MeterSig (unused)
    MeasureAligner **measureAligner = static_cast<MeasureAligner**>((*params)[0]);
    double *time = static_cast<double*>((*params)[1]);
    Mensur **currentMensur = static_cast<Mensur**>((*params)[2]);
    MeterSig **currentMeterSig = static_cast<MeterSig**>((*params)[3]);
    
    // we need to call it because we are overriding Object::AlignHorizontally
    this->ResetHorizontalAlignment();
    
    
    Chord* chordParent = dynamic_cast<Chord*>(this->GetFirstParent( CHORD, MAX_CHORD_DEPTH));
    if( chordParent )
    {
        m_alignment = chordParent->GetAlignment();
        return FUNCTOR_CONTINUE;
    }
    
    AlignmentType type = ALIGNMENT_DEFAULT;
    if (this->Is() == BAR_LINE) {
        type = ALIGNMENT_BARLINE;
    }
    else if ( this->Is() == CLEF ) {
        if ( this->GetScoreOrStaffDefAttr() ) {
            type = ALIGNMENT_CLEF_ATTR;
        }
        else {
            type = ALIGNMENT_CLEF;
        }
    }
    else if (this->Is() == KEY_SIG) {
        if ( this->GetScoreOrStaffDefAttr() ) {
            type = ALIGNMENT_KEYSIG_ATTR;
        }
        else {
            type = ALIGNMENT_KEYSIG;
        }
    }
    else if (this->Is() == MENSUR) {
        if ( this->GetScoreOrStaffDefAttr() ) {
            type = ALIGNMENT_MENSUR_ATTR;
        }
        else {
            // replace the current mensur
            (*currentMensur) = dynamic_cast<Mensur*>(this);
            assert( *currentMensur );
            type = ALIGNMENT_MENSUR;
        }
    }
    else if (this->Is() == METER_SIG) {
        if ( this->GetScoreOrStaffDefAttr() ) {
            type = ALIGNMENT_METERSIG_ATTR;
        }
        else {
            // replace the current meter signature
            (*currentMeterSig) = dynamic_cast<MeterSig*>(this);
            assert( *currentMeterSig );
            type = ALIGNMENT_METERSIG;
        }
    }
    else if ( (this->Is() == MULTI_REST) || (this->Is() == MREST) ) {
        type = ALIGNMENT_MULTIREST;
    }
    else if ( this->IsGraceNote() ) {
        type = ALIGNMENT_GRACENOTE;
    }
    else if ((this->Is() == BEAM)|| (this->Is() == TUPLET) || (this->Is() == VERSE) || (this->Is() == SYL) ) {
        type = ALIGNMENT_CONTAINER;
    }
    else if (this->Is() == DOT) {
        type = ALIGNMENT_DOT;
    }
    
    // get the duration of the event
    double duration = this->GetAlignmentDuration( *currentMensur );
    
    (*measureAligner)->SetMaxTime( (*time) + duration );
    
    m_alignment = (*measureAligner)->GetAlignmentAtTime( *time, type );
    
    if ( this->IsGraceNote() ) {
        GraceAligner *graceAligner = m_alignment->GetGraceAligner();
        graceAligner->StackNote( dynamic_cast<Note*>(this) );
    }
    
    //LogDebug("Time %f - %s", (*time), this->GetClassName().c_str() );
    
    // increase the time position
    (*time) += duration;
    
    return FUNCTOR_CONTINUE;
}
    

int LayerElement::PrepareTimeSpanning( ArrayPtrVoid *params )
{
    // param 0: std::vector<DocObject*>* that holds the current elements to match
    // param 1: bool* fillList for indicating whether the elements have to be stack or not (unused)
    std::vector<DocObject*> *elements = static_cast<std::vector<DocObject*>*>((*params)[0]);
    
    std::vector<DocObject*>::iterator iter = elements->begin();
    while ( iter != elements->end()) {
        TimeSpanningInterface *interface = dynamic_cast<TimeSpanningInterface*>(*iter);
        assert(interface);
        if (interface->SetStartAndEnd( this ) ) {
            iter = elements->erase( iter );
        }
        else {
            iter++;
        }
    }
    
    return FUNCTOR_CONTINUE;
}
    
int LayerElement::SetDrawingXY( ArrayPtrVoid *params )
{
    // param 0: a pointer doc
    // param 1: a pointer to the current system (unused)
    // param 2: a pointer to the current measure
    // param 3: a pointer to the current staff
    // param 4: a pointer to the current layer
    // param 5: a pointer to the view
    // param 6: a bool indicating if we are processing layer elements or not
    Doc *doc = static_cast<Doc*>((*params)[0]);
    Measure **currentMeasure = static_cast<Measure**>((*params)[2]);
    Staff **currentStaff = static_cast<Staff**>((*params)[3]);
    Layer **currentLayer = static_cast<Layer**>((*params)[4]);
    View *view = static_cast<View*>((*params)[5]);
    bool *processLayerElements = static_cast<bool*>((*params)[6]);
    
    // First pass, only set the X position
    if ((*processLayerElements)==false) {
        // Here we set the appropriate x value to be used for drawing
        // With Raw documents, we use m_drawingXRel that is calculated by the layout algorithm
        // With Transcription documents, we use the m_xAbs
        if ( this->m_xAbs == VRV_UNSET ) {
            assert( doc->GetType() == Raw );
            this->SetDrawingX( this->GetXRel() + (*currentMeasure)->GetDrawingX() );
            // Grace notes, also take into account the GraceAlignment
            Note *note = dynamic_cast<Note*>(this);
            if (note && note->HasGraceAlignment() ) {
                this->SetDrawingX( this->GetDrawingX() - note->GetAlignment()->GetGraceAligner()->GetWidth()
                                  + note->GetGraceAlignment()->GetXRel() );
            }
        }
        else
        {
            assert( doc->GetType() == Transcription );
            this->SetDrawingX( this->m_xAbs );
        }
        return FUNCTOR_CONTINUE;
    }
    
    LayerElement *layerElementY = this;
    
    // Look for cross-staff situations
    // If we have one, make is available in m_crossStaff
    DurationInterface *durElement = dynamic_cast<DurationInterface*>(this);
    if ( durElement && durElement->HasStaff()) {
        AttCommonNComparison comparisonFirst( STAFF, durElement->GetStaff() );
        m_crossStaff = dynamic_cast<Staff*>((*currentMeasure)->FindChildByAttComparison(&comparisonFirst, 1));
        if (m_crossStaff) {
            if (m_crossStaff == (*currentStaff)) LogWarning("The cross staff reference '%d' for element '%s' seems to be identical to the parent staff", durElement->GetStaff(), this->GetUuid().c_str());
            // Now try to get the corresponding layer - for now look for the same layer @n
            int layerN = (*currentLayer)->GetN();
            // When we will have allowed @layer in <note>, we will have to do:
            // int layerN = durElement->HasLayer() ? durElement->GetLayer() : (*currentLayer)->GetN();
            AttCommonNComparison comparisonFirstLayer( LAYER, layerN );
            m_crossLayer = dynamic_cast<Layer*>(m_crossStaff->FindChildByAttComparison(&comparisonFirstLayer, 1));
            if (m_crossLayer) {
                // Now we need to yet the element at the same position in the cross-staff layer of getting the right clef
                layerElementY = m_crossLayer->GetAtPos( this->GetDrawingX() );
                
            } else {
                LogWarning("Could not get the layer with cross-staff reference '%d' for element '%s'", durElement->GetStaff(), this->GetUuid().c_str());
            }
        } else {
            LogWarning("Could not get the cross staff reference '%d' for element '%s'", durElement->GetStaff(), this->GetUuid().c_str());
        }
        // If we have a @layer we probably also want to change the layer element (for getting the right clef if different)
    } else {
        m_crossStaff = NULL;
        m_crossLayer = NULL;
    }
    
    Staff *staffY = m_crossStaff ? m_crossStaff : (*currentStaff);
    Layer *layerY = m_crossLayer ? m_crossLayer : (*currentLayer);
    
    // Here we set the appropriate Y value to be used for drawing
    if ( this->m_xAbs == VRV_UNSET ) {
        assert( doc->GetType() == Raw );
        this->SetDrawingY( staffY->GetDrawingY() );
    }
    else
    {
        assert( doc->GetType() == Transcription );
        this->SetDrawingY( staffY->GetDrawingY() );
    }
    
    // Finally, adjust Y for notes and rests
    if (this->Is() == NOTE)
    {
        Note *note = dynamic_cast<Note*>(this);
        assert( note );
        this->SetDrawingY( this->GetDrawingY() + view->CalculatePitchPosY( staffY, note->GetPname(), layerY->GetClefOffset( layerElementY ), note->GetOct() ) );
    }
    else if (this->Is() == REST) {
        Rest *rest = dynamic_cast<Rest*>(this);
        assert( rest );
        // Automatically calculate rest position, if so requested
        if (rest->GetPloc() == PITCHNAME_NONE) {
            this->SetDrawingY( this->GetDrawingY() + view->CalculateRestPosY( staffY, rest->GetActualDur()) );
        } else {
            this->SetDrawingY( this->GetDrawingY() + view->CalculatePitchPosY( staffY, rest->GetPloc(), layerY->GetClefOffset( layerElementY ), rest->GetOloc()) );
        }
    }
    
    return FUNCTOR_CONTINUE;
}

} // namespace vrv
