/////////////////////////////////////////////////////////////////////////////
// Name:        drawinglistinterface.cpp
// Author:      Laurent Pugin
// Created:     2015
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#include "drawinglistinterface.h"

//----------------------------------------------------------------------------

#include "layerelement.h"

namespace vrv {

//----------------------------------------------------------------------------
// DrawingListInterface
//----------------------------------------------------------------------------

DrawingListInterface::DrawingListInterface()
{
    Reset();
}


DrawingListInterface::~DrawingListInterface()
{
}
    
    
void DrawingListInterface::Reset()
{
    m_drawingList.clear();
}
    
    
    
void DrawingListInterface::AddToDrawingList( DocObject *object )
{
    m_drawingList.push_back( object );
    m_drawingList.sort();
    m_drawingList.unique();
}

ListOfObjects *DrawingListInterface::GetDrawingList( )
{
    return &m_drawingList;
}

void DrawingListInterface::ResetDrawingList( )
{
    m_drawingList.clear();
}

//----------------------------------------------------------------------------
// KeySigDrawingInterface
//----------------------------------------------------------------------------

KeySigDrawingInterface::KeySigDrawingInterface()
{
    Reset();
}


KeySigDrawingInterface::~KeySigDrawingInterface()
{
}


void KeySigDrawingInterface::Reset()
{
    m_drawingCancelAccidType = ACCIDENTAL_EXPLICIT_n;
    m_drawingCancelAccidCount = 0;
    m_drawingShow = true;
    m_drawingShowchange = false;
}

} // namespace vrv
