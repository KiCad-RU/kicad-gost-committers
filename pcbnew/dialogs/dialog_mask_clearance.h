/////////////////////////////////////////////////////////////////////////////

// Name:        dialog_track_options.h
// Author:      jean-pierre Charras
// Created:     17 feb 2009
// Licence: GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef _DIALOG_MASK_CLEARANCE_H_
#define _DIALOG_MASK_CLEARANCE_H_

#include <dialog_mask_clearance_base.h>

/**
 *  DIALOG_PADS_MASK_CLEARANCE, derived from DIALOG_PADS_MASK_CLEARANCE_BASE
 *  @see dialog_mask_clearance.h and dialog_mask_clearance.cpp,
 *  automatically created by wxFormBuilder
 */
class DIALOG_PADS_MASK_CLEARANCE : public DIALOG_PADS_MASK_CLEARANCE_BASE
{
private:
    PCB_EDIT_FRAME*  m_parent;
    BOARD_DESIGN_SETTINGS  m_brdSettings;

public:
    DIALOG_PADS_MASK_CLEARANCE( PCB_EDIT_FRAME* parent );
    ~DIALOG_PADS_MASK_CLEARANCE() {};
private:
    void         myInit();
    virtual void OnButtonOkClick( wxCommandEvent& event );
    virtual void OnButtonCancelClick( wxCommandEvent& event );
};

#endif    // _DIALOG_MASK_CLEARANCE_H_
