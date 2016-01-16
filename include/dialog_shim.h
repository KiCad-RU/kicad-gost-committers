/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2012 KiCad Developers, see CHANGELOG.TXT for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef DIALOG_SHIM_
#define DIALOG_SHIM_

#include <wx/dialog.h>
#include <hashtables.h>
#include <kiway_player.h>

#define DLGSHIM_USE_SETFOCUS      0

class WDO_ENABLE_DISABLE;
class EVENT_LOOP;

// These macros are for DIALOG_SHIM only, NOT for KIWAY_PLAYER.  KIWAY_PLAYER
// has its own support for quasi modal and its platform specific issues are different
// than for a wxDialog.
 #define SHOWQUASIMODAL     ShowQuasiModal
 #define ENDQUASIMODAL      EndQuasiModal


/**
 * Class DIALOG_SHIM
 * may sit in the inheritance tree between wxDialog and any class written by
 * wxFormBuilder.  To put it there, use wxFormBuilder tool and set:
 * <br> subclass name = DIALOG_SHIM
 * <br> subclass header = dialog_shim.h
 * <br>
 * in the dialog window's properties.
 **/
class DIALOG_SHIM : public wxDialog, public KIWAY_HOLDER
{
public:

    DIALOG_SHIM( wxWindow* aParent, wxWindowID id, const wxString& title,
            const   wxPoint& pos = wxDefaultPosition,
            const   wxSize&  size = wxDefaultSize,
            long    style = wxDEFAULT_DIALOG_STYLE,
            const   wxString& name = wxDialogNameStr
            );

    ~DIALOG_SHIM();

    int ShowQuasiModal();      // disable only the parent window, otherwise modal.

    void EndQuasiModal( int retCode );  // End quasi-modal mode

    bool IsQuasiModal()         { return m_qmodal_showing; }

    bool Show( bool show );     // override wxDialog::Show

    bool Enable( bool enable ); // override wxDialog::Enable virtual

protected:

    std::string m_hash_key;     // alternate for class_map when classname re-used.

    // variables for quasi-modal behavior support, only used by a few derivatives.
    EVENT_LOOP*         m_qmodal_loop;      // points to nested event_loop, NULL means not qmodal and dismissed
    bool                m_qmodal_showing;
    WDO_ENABLE_DISABLE* m_qmodal_parent_disabler;

#if DLGSHIM_USE_SETFOCUS
private:
    void    onInit( wxInitDialogEvent& aEvent );
#endif
};

#endif  // DIALOG_SHIM_
