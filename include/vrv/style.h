/////////////////////////////////////////////////////////////////////////////
// Name:        style.h
// Author:      Laurent Pugin
// Created:     2005
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#ifndef __VRV_STYLE_H__
#define __VRV_STYLE_H__

namespace vrv {
    
//----------------------------------------------------------------------------
// Default layout values
//----------------------------------------------------------------------------

#define DEFAULT_UNIT 9
#define MIN_UNIT 6
#define MAX_UNIT 18
    
#define DEFAULT_PAGE_LEFT_MAR 50
#define MIN_PAGE_LEFT_MAR 0
#define MAX_PAGE_LEFT_MAR 500
    
#define DEFAULT_PAGE_RIGHT_MAR 50
#define MIN_PAGE_RIGHT_MAR 0
#define MAX_PAGE_RIGHT_MAR 500
    
#define DEFAULT_PAGE_TOP_MAR 50
#define MIN_PAGE_TOP_MAR 0
#define MAX_PAGE_TOP_MAR 500
    
#define DEFAULT_PAGE_HEIGHT 2970
#define MIN_PAGE_HEIGHT 100
#define MAX_PAGE_HEIGHT 60000
    
#define DEFAULT_PAGE_WIDTH 2100
#define MIN_PAGE_WIDTH 100
#define MAX_PAGE_WIDTH 60000
    
#define DEFAULT_BARLINE_WITDH 3.0
#define MIN_BARLINE_WIDTH 1.0
#define MAX_BARLINE_WIDTH 8.0
    
#define DEFAULT_STAFFLINE_WITDH 2.0
#define MIN_STAFFLINE_WIDTH 1.0
#define MAX_STAFFLINE_WIDTH 8.0
    
#define DEFAULT_STEM_WITDH 2.0
#define MIN_STEM_WIDTH 1.0
#define MAX_STEM_WIDTH 5.0
    
#define MIN_TIE_HEIGHT 20
#define MIN_TIE_THICKNESS 6
    
//----------------------------------------------------------------------------
// Default scaling (%) and spacing (units) values
//----------------------------------------------------------------------------
    
#define DEFAULT_SCALE 100
#define MIN_SCALE 1
#define MAX_SCALE 1000
    
#define DEFAULT_SPACING_STAFF 8
#define MIN_SPACING_STAFF 0
#define MAX_SPACING_STAFF 12
    
#define DEFAULT_SPACING_SYSTEM 0
#define MIN_SPACING_SYSTEM 0
#define MAX_SPACING_SYSTEM 12
    
#define DEFAULT_LYRIC_SIZE 4.5
#define MIN_LYRIC_SIZE 2.0
#define MAX_LYRIC_SIZE 8.0
    
#define DEFAULT_MEASURE_WIDTH 3.0
#define MIN_MEASURE_WIDTH 1.0
#define MAX_MEASURE_WIDTH 30.0
    
/** Left margins */
    
#define DEFAULT_LEFT_MARGIN_BARLINE 0.5
#define MIN_LEFT_MARGIN_BARLINE -10.0
#define MAX_LEFT_MARGIN_BARLINE 10.0
    
#define DEFAULT_LEFT_MARGIN_BARLINE_ATTR 2.5
#define MIN_LEFT_MARGIN_BARLINE_ATTR -10.0
#define MAX_LEFT_MARGIN_BARLINE_ATTR 10.0
    
#define DEFAULT_LEFT_MARGIN_CHORD 1.0
#define MIN_LEFT_MARGIN_CHORD -10.0
#define MAX_LEFT_MARGIN_CHORD 10.0
    
#define DEFAULT_LEFT_MARGIN_CLEF -2.0
#define MIN_LEFT_MARGIN_CLEF -10.0
#define MAX_LEFT_MARGIN_CLEF 10.0
    
#define DEFAULT_LEFT_MARGIN_MREST 3.0
#define MIN_LEFT_MARGIN_MREST -10.0
#define MAX_LEFT_MARGIN_MREST 10.0
    
#define DEFAULT_LEFT_MARGIN_NOTE 1.0
#define MIN_LEFT_MARGIN_NOTE -10.0
#define MAX_LEFT_MARGIN_NOTE 10.0
    
#define DEFAULT_LEFT_MARGIN_DEFAULT 0.0
#define MIN_LEFT_MARGIN_DEFAULT -10.0
#define MAX_LEFT_MARGIN_DEFAULT 10.0
    
/** Right margins */
    
#define DEFAULT_RIGHT_MARGIN_BARLINE 3.0
#define MIN_RIGHT_MARGIN_BARLINE 0.0
#define MAX_RIGHT_MARGIN_BARLINE 10.0
    
#define DEFAULT_RIGHT_MARGIN_BARLINE_ATTR 0.0
#define MIN_RIGHT_MARGIN_BARLINE_ATTR 0.0
#define MAX_RIGHT_MARGIN_BARLINE_ATTR 10.0
    
#define DEFAULT_RIGHT_MARGIN_CLEF 2.0
#define MIN_RIGHT_MARGIN_CLEF 0.0
#define MAX_RIGHT_MARGIN_CLEF 10.0
    
#define DEFAULT_RIGHT_MARGIN_KEYSIG 2.5
#define MIN_RIGHT_MARGIN_KEYSIG 0.0
#define MAX_RIGHT_MARGIN_KEYSIG 10.0
    
#define DEFAULT_RIGHT_MARGIN_MENSUR 3.0
#define MIN_RIGHT_MARGIN_MENSUR 0.0
#define MAX_RIGHT_MARGIN_MENSUR 10.0
    
#define DEFAULT_RIGHT_MARGIN_METERSIG 3.0
#define MIN_RIGHT_MARGIN_METERSIG 0.0
#define MAX_RIGHT_MARGIN_METERSIG 10.0
    
#define DEFAULT_RIGHT_MARGIN_MREST 3.0
#define MIN_RIGHT_MARGIN_MREST 0.0
#define MAX_RIGHT_MARGIN_MREST 10.0
    
#define DEFAULT_RIGHT_MARGIN_MULTIREST 0.5
#define MIN_RIGHT_MARGIN_MULTIREST 0.0
#define MAX_RIGHT_MARGIN_MULTIREST 10.0
    
#define DEFAULT_RIGHT_MARGIN_DEFAULT 1.0
#define MIN_RIGHT_MARGIN_DEFAULT 0.0
#define MAX_RIGHT_MARGIN_DEFAULT 10.0
    
//----------------------------------------------------------------------------
// Style
//----------------------------------------------------------------------------
    
// the space between the staff and an editorial accid in units
#define TEMP_STYLE_ACCID_EDIT_SPACE 3.5 * PARAM_DENOMINATOR

// the space between each lyric line in units
#define TEMP_STYLE_LYIRC_LINE_SPACE 5.0 * PARAM_DENOMINATOR


/**
 * This class contains the document default environment variables.
 * Some of them are not available as is in MEI - to be solved
 */
class Style
{
public:
    // constructors and destructors
    Style();
    virtual ~Style();
    
    
public:
    /** The unit (1�2 of the distance between staff lines) **/
    int m_unit;
    /** The landscape paper orientation flag */
    char m_landscape;
    /** The staff line width */
    unsigned short m_staffLineWidth;
    /** The stem width */
    unsigned short m_stemWidth;
    /** The barLine width */
    unsigned short m_barLineWidth;
    /** The maximum beam slope */
    unsigned char m_beamMaxSlope;
    /** The minimum beam slope */
    unsigned char m_beamMinSlope;
    /** The grace size ratio numerator */
    unsigned char m_graceNum;
    /** The grace size ratio denominator */
    unsigned char m_graceDen;
    /** The page height */
    int m_pageHeight;
    /** The page width */
    int m_pageWidth;
    /** The page left margin */
    short m_pageLeftMar;
    /** The page right margin */
    short m_pageRightMar;
    /** The page top margin */
    short m_pageTopMar;
    /** The staff minimal spacing */
    short m_spacingStaff;
    /** The system minimal spacing */
    short m_spacingSystem;
    
    /** The minimal measure width in units / PARAM_DENOMINATOR */
    short m_minMeasureWidth;
    /** The lyrics size (in units / PARAM_DENOMINATOR) */
    int m_lyricSize;
    
    /** The layout left margin by element */
    char m_leftMarginBarline;
    char m_leftMarginBarlineAttr;
    char m_leftMarginChord;
    char m_leftMarginClef;
    char m_leftMarginMRest;
    char m_leftMarginNote;
    /** The default left margin */
    char m_leftMarginDefault;
    
    /** The layout right margin by element */
    char m_rightMarginClef;
    char m_rightMarginKeySig;
    char m_rightMarginMensur;
    char m_rightMarginMeterSig;
    char m_rightMarginBarline;
    char m_rightMarginBarlineAttr;
    char m_rightMarginMRest;
    char m_rightMarginMultiRest;
    /** The default right margin */
    char m_rightMarginDefault;
    
};


} // namespace vrv

#endif // __VRV_DEF_H__