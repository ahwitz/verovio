/////////////////////////////////////////////////////////////////////////////
// Name:        accid.cpp
// Author:      Laurent Pugin
// Created:     2014
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "accid.h"

//----------------------------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------------------------

#include "note.h"
#include "smufl.h"
#include "vrv.h"

namespace vrv {

//----------------------------------------------------------------------------
// Accid
//----------------------------------------------------------------------------

Accid::Accid() : LayerElement("accid-"), PositionInterface(), AttAccidental(), AttAccidLog()
{

    RegisterInterface(PositionInterface::GetAttClasses(), PositionInterface::IsInterface());
    RegisterAttClass(ATT_ACCIDENTAL);
    RegisterAttClass(ATT_ACCIDLOG);

    Reset();
}

Accid::~Accid()
{
}

void Accid::Reset()
{
    LayerElement::Reset();
    PositionInterface::Reset();
    ResetAccidental();
    ResetAccidLog();
}

//----------------------------------------------------------------------------
// Functor methods
//----------------------------------------------------------------------------

int Accid::PreparePointersByLayer(ArrayPtrVoid *params)
{
    // param 0: the current Note (not used)
    // Note **currentNote = static_cast<Note**>((*params).at(0));

    Note *note = dynamic_cast<Note *>(this->GetFirstParent(NOTE, MAX_ACCID_DEPTH));
    if (!note) {
        return FUNCTOR_CONTINUE;
    }

    if (note->m_drawingAccid != NULL) {
        note->ResetDrawingAccid();
    }
    note->m_drawingAccid = this;

    return FUNCTOR_CONTINUE;
}
    
std::vector<int>* Accid::GetGlyphs()
{
    // Insert glyphs into the vector in reverse order
    this->glyphs.clear();
    
    switch (this->GetAccid()) {
        case ACCIDENTAL_EXPLICIT_s:
            this->glyphs.push_back(SMUFL_E262_accidentalSharp);
            break;
        case ACCIDENTAL_EXPLICIT_f:
            this->glyphs.push_back(SMUFL_E260_accidentalFlat);
            break;
        case ACCIDENTAL_EXPLICIT_x:
            this->glyphs.push_back(SMUFL_E263_accidentalDoubleSharp);
            break;
        case ACCIDENTAL_EXPLICIT_ss:
            this->glyphs.push_back(SMUFL_E262_accidentalSharp);
            this->glyphs.push_back(SMUFL_E262_accidentalSharp);
            break;
        case ACCIDENTAL_EXPLICIT_ff:
            this->glyphs.push_back(SMUFL_E260_accidentalFlat);
            this->glyphs.push_back(SMUFL_E260_accidentalFlat);
            break;
        case ACCIDENTAL_EXPLICIT_n:
            this->glyphs.push_back(SMUFL_E261_accidentalNatural);
            break;
        case ACCIDENTAL_EXPLICIT_nf:
            this->glyphs.push_back(SMUFL_E260_accidentalFlat);
            this->glyphs.push_back(SMUFL_E261_accidentalNatural);
            break;
        case ACCIDENTAL_EXPLICIT_ns:
            this->glyphs.push_back(SMUFL_E262_accidentalSharp);
            this->glyphs.push_back(SMUFL_E261_accidentalNatural);
            break;
        case ACCIDENTAL_EXPLICIT_xs:
            this->glyphs.push_back(SMUFL_E262_accidentalSharp);
            this->glyphs.push_back(SMUFL_E263_accidentalDoubleSharp);
            break;
        case ACCIDENTAL_EXPLICIT_tf:
            this->glyphs.push_back(SMUFL_E260_accidentalFlat);
            this->glyphs.push_back(SMUFL_E260_accidentalFlat);
            this->glyphs.push_back(SMUFL_E260_accidentalFlat);
            break;
        case ACCIDENTAL_EXPLICIT_ts:
            this->glyphs.push_back(SMUFL_E262_accidentalSharp);
            this->glyphs.push_back(SMUFL_E262_accidentalSharp);
            this->glyphs.push_back(SMUFL_E262_accidentalSharp);
            break;
//        case ACCIDENTAL_EXPLICIT_sx:
//            this->glyphs.push_back(SMUFL_E262_accidentalSharp);
//            this->glyphs.push_back(SMUFL_E263_accidentalDoubleSharp);
//            break;
//        case ACCIDENTAL_EXPLICIT_su:
//            this->glyphs.push_back(SMUFL_E274_accidentalThreeQuartersSharpArrowUp);
//            break;
//        case ACCIDENTAL_EXPLICIT_sd:
//            this->glyphs.push_back(SMUFL_E275_accidentalQuarterSharpArrowDown);
//            break;
//        case ACCIDENTAL_EXPLICIT_fu:
//            this->glyphs.push_back(SMUFL_E270_accidentalQuarterFlatArrowUp);
//            break;
//        case ACCIDENTAL_EXPLICIT_fd:
//            this->glyphs.push_back(SMUFL_E271_accidentalThreeQuartersFlatArrowDown);
//            break;
//        case ACCIDENTAL_EXPLICIT_nu:
//            this->glyphs.push_back(SMUFL_E272_accidentalQuarterSharpNaturalArrowUp);
//            break;
//        case ACCIDENTAL_EXPLICIT_nd:
//            this->glyphs.push_back(SMUFL_E273_accidentalQuarterFlatNaturalArrowDown);
//            break;
//        case ACCIDENTAL_EXPLICIT_1qf:
//            this->glyphs.push_back(SMUFL_E280_accidentalQuarterToneFlatStein);
//            break;
//        case ACCIDENTAL_EXPLICIT_3qf:
//            this->glyphs.push_back(SMUFL_E281_accidentalThreeQuarterTonesFlatZimmermann);
//            break;
//        case ACCIDENTAL_EXPLICIT_1qs:
//            this->glyphs.push_back(SMUFL_E282_accidentalQuarterToneSharpStein);
//            break;
//        case ACCIDENTAL_EXPLICIT_3qs:
//            this->glyphs.push_back(SMUFL_E283_accidentalThreeQuarterTonesSharpStein);
//            break;
        default:
            LogWarning("Accidental '%s' can not be rendered as a vector yet.", AttAccidental::AccidentalExplicitToStr(this->GetAccid()).c_str());
            break;
    }
    
    return &(this->glyphs);
}

} // namespace vrv
