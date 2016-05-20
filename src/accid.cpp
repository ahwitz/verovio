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
    
int Accid::GetGlyphs()
{
    int glyphNums = 0;
    switch (this->GetAccid())
    {
        case ACCIDENTAL_EXPLICIT_s:
        case ACCIDENTAL_EXPLICIT_f:
        case ACCIDENTAL_EXPLICIT_x:
        case ACCIDENTAL_EXPLICIT_n:
        case ACCIDENTAL_EXPLICIT_1qf:
        case ACCIDENTAL_EXPLICIT_3qf:
        case ACCIDENTAL_EXPLICIT_1qs:
        case ACCIDENTAL_EXPLICIT_3qs:
        case ACCIDENTAL_EXPLICIT_su:
        case ACCIDENTAL_EXPLICIT_sd:
        case ACCIDENTAL_EXPLICIT_fu:
        case ACCIDENTAL_EXPLICIT_fd:
        case ACCIDENTAL_EXPLICIT_nu:
        case ACCIDENTAL_EXPLICIT_nd:
            glyphNums = 1;
            break;
            
        case ACCIDENTAL_EXPLICIT_ss:
        case ACCIDENTAL_EXPLICIT_ff:
        case ACCIDENTAL_EXPLICIT_xs:
        case ACCIDENTAL_EXPLICIT_nf:
        case ACCIDENTAL_EXPLICIT_ns:
            glyphNums = 2;
            break;
        
        case ACCIDENTAL_EXPLICIT_ts:
        case ACCIDENTAL_EXPLICIT_tf:
            glyphNums = 3;
            break;
            
        default:
            glyphNums = 0;
            break;
    }
    return glyphNums;
}

} // namespace vrv
