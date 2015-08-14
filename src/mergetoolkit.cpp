/////////////////////////////////////////////////////////////////////////////
// Name:        toolkit.cpp
// Author:      Laurent Pugin
// Created:     17/10/2013
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#include "mergetoolkit.h"

//----------------------------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------------------------

#include "editorial.h"
#include "iodarms.h"
#include "iomei.h"
#include "iopae.h"
#include "layer.h"
#include "page.h"
#include "measure.h"
#include "note.h"
#include "rest.h"
#include "slur.h"
#include "staff.h"
#include "svgdevicecontext.h"
#include "style.h"
#include "system.h"
#include "verse.h"
#include "vrv.h"

namespace vrv {
    
    
    MergeToolkit::MergeToolkit( bool initFont ):Toolkit(initFont)
    {
        
    }
    
    void MergeToolkit::MergeMeasures(Measure* m1, Measure* m2)
    {
            // may be needed for the future
    }

    void MergeToolkit::MergeStaffs(Staff* s1, Staff* s2)
    {
        // assume 1 layer
        Layer* layer_1 = dynamic_cast<Layer*>(s1->m_children.front());
        Layer* layer_2 = dynamic_cast<Layer*>(s2->m_children.front());
        if (layer_1 != NULL && layer_2 != NULL) {
            MergeLayers(layer_1, layer_2);
        }
    }
    
    void MergeToolkit::MergeLayers(Layer* l1, Layer* l2)
    {
        ArrayOfObjects children_1 = l1->m_children;
        ArrayOfObjects children_2 = l2->m_children;
        int smaller = (int)(std::min(children_1.size(), children_2.size()));
        for (int k = 0; k < smaller; k++) {
            Note* note_1 = dynamic_cast<Note*>(children_1.at(k));
            Note* note_2 = dynamic_cast<Note*>(children_2.at(k));
            if (note_1 == NULL || note_2 == NULL)
            {
                Beam* beam_1 = dynamic_cast<Beam*>(children_1.at(k));
                Beam* beam_2 = dynamic_cast<Beam*>(children_2.at(k));

                if (beam_1 != NULL && beam_2 != NULL) {

                    MergeBeams(beam_1, beam_2);
                }
            }
            else
            {
                MergeNotes(note_1, note_2);
            }
        }
        
    }
    
    void MergeToolkit::MergeBeams(Beam* b1, Beam* b2)
    {
        int smaller = (int)(std::min(b1->m_children.size(), b2->m_children.size()));
        for (int k = 0; k < smaller; k++) {
            Note* note_1 = dynamic_cast<Note*>(b1->m_children.at(k));
            Note* note_2 = dynamic_cast<Note*>(b2->m_children.at(k));
            if (note_1 == NULL || note_2 == NULL)
            {
                if (dynamic_cast<Beam*>(note_1) != NULL && dynamic_cast<Beam*>(note_2) != NULL) {
                }
            }
            else
            {
                MergeNotes(note_1, note_2);
            }
        }
        
        
        
    }
    
    void MergeToolkit::MergeNotes(Note* n1, Note* n2)
    {
            if (n1->GetActualDur() == n2->GetActualDur() &&
                n1->GetDiatonicPitch() == n2->GetDiatonicPitch()) {
                if (n1->m_children.size() > 0 && n1->m_children.size() > 0) {
                    Verse* verse_1 = dynamic_cast<Verse*>(n1->m_children.front());
                    Verse* verse_2 = dynamic_cast<Verse*>(n2->m_children.front());
                    if (verse_1 != NULL && verse_2 != NULL) {
                        MergeVerses(verse_1, verse_2, n1, n2, 0);
                    }
                    else {
                        verse_1 = dynamic_cast<Verse*>(n1->m_children.back());
                        verse_2 = dynamic_cast<Verse*>(n2->m_children.back());
                        if (verse_1 != NULL && verse_2 != NULL) {
                            MergeVerses(verse_1, verse_2, n1, n2, 1);
                        }
                    }
                }
            }

        
    }

    void MergeToolkit::MergeRests(Rest* r1, Rest* r2)
    {
        
    }
    
    void MergeToolkit::MergeVerses(Verse* v1, Verse* v2, Note* n1, Note* n2, int index)
    {
        if (v1 != NULL && v2!= NULL) {
            n1->DetachChild(index);
            n2->DetachChild(index);
            v1->m_parent = NULL;
            v2->m_parent = NULL;
            App* app = new App(EDITORIAL_NOTE);
            Lem* lem = new Lem();
            lem->AddLayerElement(v1);
            Rdg* rdg = new Rdg();
            rdg->AddLayerElement(v2);
            app->AddLemOrRdg(lem);
            app->AddLemOrRdg(rdg);
            n1->AddEditorialElement(app);
        }
    }
    
    
    std::string MergeToolkit::Merge(int pageNo)
    {
        pageNo--;
        Page* current_page = dynamic_cast<Page*>((&m_doc)->GetChild(pageNo));
        ArrayOfObjects systems = current_page->m_children;
        for (int i = 0; i < systems.size(); i++) {
            System* current_system = dynamic_cast<System*>(systems.at(i));
            ArrayOfObjects measures = current_system->m_children;
            for(int j = 0; j < measures.size(); j++) {
                // for now assume two voices/staffs
                Measure* current_measure = dynamic_cast<Measure*>(measures.at(j));
                ArrayOfObjects staffs = current_measure->m_children;
                Staff* staff_1 = dynamic_cast<Staff*>(staffs.at(0));
                Staff* staff_2 = dynamic_cast<Staff*>(staffs.at(1));
                MergeStaffs(staff_1, staff_2);
                staff_2->m_parent = NULL;
                current_measure->DetachChild(1);
                //delete staff_2;
            }
        }
        
        MeiOutput meioutput( &m_doc, "" );
        meioutput.SetScoreBasedMEI( true);
        return meioutput.GetOutput( pageNo );
    }
 } //namespace vrv
