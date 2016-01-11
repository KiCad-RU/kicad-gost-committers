/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Cirilo Bernardo <cirilo.bernardo@gmail.com>
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

/**
 * @file dialog_config_3dpath.h
 * creates a dialog to edit the default search paths for 3D model files.
 */

#ifndef DIALOG_CONFIG_3DPATH_H
#define DIALOG_CONFIG_3DPATH_H

#include <vector>
#include <wx/wx.h>
#include <wx/listctrl.h>

class S3D_FILENAME_RESOLVER;

class DLG_CFG_3DPATH : public wxDialog
{
private:
    wxListView* pathList;
    wxButton* editButton;
    wxButton* deleteButton;

    S3D_FILENAME_RESOLVER* resolver;
    std::vector< wxString > m_paths;

public:
    DLG_CFG_3DPATH( wxWindow* aParent, S3D_FILENAME_RESOLVER* aResolver );
    virtual bool TransferDataFromWindow();

private:
    void EditPath( wxCommandEvent& event );
    void AddPath( wxCommandEvent& event );
    void DeletePath( wxCommandEvent& event );
    void PathSelect( wxCommandEvent& event );

    wxDECLARE_EVENT_TABLE();
};

#endif  // DIALOG_CONFIG_3DPATH_H
