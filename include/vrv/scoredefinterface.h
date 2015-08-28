/////////////////////////////////////////////////////////////////////////////
// Name:        scoredefinterface.h
// Author:      Laurent Pugin
// Created:     2015
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#ifndef __VRV_SCORE_DEF_INTERFACE_H__
#define __VRV_SCORE_DEF_INTERFACE_H__

#include "atts_mensural.h"
#include "atts_shared.h"
#include "vrvdef.h"

namespace vrv {

//----------------------------------------------------------------------------
// ScoreDefInterface
//----------------------------------------------------------------------------

/** 
 * This class is an interface for elements implementing score attributes, such
 * as <scoreDef>, or <staffDef>
 * It is not an abstract class but should not be instanciate directly.
 */
class ScoreDefInterface: public Interface,
    public AttCleffingLog,
    public AttKeySigDefaultLog,
    public AttKeySigDefaultVis,
    public AttMensuralLog,
    public AttMensuralShared,
    public AttMeterSigDefaultLog,
    public AttMeterSigDefaultVis,
    public AttMultinummeasures
{
public:
    /**
     * @name Constructors, destructors, reset methods
     * Reset method reset all attribute classes
     */
    ///@{
    ScoreDefInterface();
    virtual ~ScoreDefInterface();
    virtual void Reset();
    virtual InterfaceId IsInterface() { return INTERFACE_SCOREDEF; };
    ///@}
   
protected:
    
private:
    
public:

private:
    
};
    
} // namespace vrv 

#endif
