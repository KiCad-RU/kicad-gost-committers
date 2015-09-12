/**
 * @file dialog_export_vrml.cpp
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009-2013  Lorenzo Mercantonio
 * Copyright (C) 2013 Jean-Pierre Charras jp.charras at wanadoo.fr
 * Copyright (C) 2004-2015 KiCad Developers, see change_log.txt for contributors.
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
#include <fctsys.h>
#include <wxPcbStruct.h>
#include <kiface_i.h>
#include <pcbnew.h>
#include <class_board.h>


/* the dialog to create VRML files, derived from DIALOG_EXPORT_3DFILE_BASE,
 * created by wxFormBuilder
 */
#include <dialog_export_vrml_base.h> // the wxFormBuilder header file

#define OPTKEY_OUTPUT_UNIT wxT( "VrmlExportUnit" )
#define OPTKEY_3DFILES_OPT wxT( "VrmlExportCopyFiles" )
#define OPTKEY_USE_RELATIVE_PATHS wxT( "VrmlUseRelativePaths" )
#define OPTKEY_USE_PLAIN_PCB wxT( "VrmlUsePlainPCB" )
#define OPTKEY_VRML_REF_UNITS wxT( "VrmlRefUnits" )
#define OPTKEY_VRML_REF_X wxT( "VrmlRefX" )
#define OPTKEY_VRML_REF_Y wxT( "VrmlRefY" )


class DIALOG_EXPORT_3DFILE : public DIALOG_EXPORT_3DFILE_BASE
{
private:
    PCB_EDIT_FRAME* m_parent;
    wxConfigBase*   m_config;
    int             m_unitsOpt;             // Remember last units option
    bool            m_copy3DFilesOpt;       // Remember last copy model files option
    bool            m_useRelativePathsOpt;  // Remember last use absolute paths option
    bool            m_usePlainPCBOpt;       // Remember last Plain Board option
    int             m_RefUnits;             // Remember last units for Reference Point
    double          m_XRef;                 // Remember last X Reference Point
    double          m_YRef;                 // Remember last Y Reference Point

public:
    DIALOG_EXPORT_3DFILE( PCB_EDIT_FRAME* parent ) :
        DIALOG_EXPORT_3DFILE_BASE( parent )
    {
        m_parent = parent;
        m_config = Kiface().KifaceSettings();
        m_filePicker->SetFocus();
        m_config->Read( OPTKEY_OUTPUT_UNIT, &m_unitsOpt, 1 );
        m_config->Read( OPTKEY_3DFILES_OPT, &m_copy3DFilesOpt, false );
        m_config->Read( OPTKEY_USE_RELATIVE_PATHS, &m_useRelativePathsOpt, false );
        m_config->Read( OPTKEY_USE_PLAIN_PCB, &m_usePlainPCBOpt, false );
        m_config->Read( OPTKEY_VRML_REF_UNITS, &m_RefUnits, 0 );
        m_config->Read( OPTKEY_VRML_REF_X, &m_XRef, 0.0 );
        m_config->Read( OPTKEY_VRML_REF_Y, &m_YRef, 0.0 );
        m_rbSelectUnits->SetSelection( m_unitsOpt );
        m_cbCopyFiles->SetValue( m_copy3DFilesOpt );
        m_cbUseRelativePaths->SetValue( m_useRelativePathsOpt );
        m_cbPlainPCB->SetValue( m_usePlainPCBOpt );
        m_VRML_RefUnitChoice->SetSelection( m_RefUnits );
        wxString tmpStr;
        tmpStr << m_XRef;
        m_VRML_Xref->SetValue( tmpStr );
        tmpStr = wxT( "" );
        tmpStr << m_YRef;
        m_VRML_Yref->SetValue( tmpStr );
        m_sdbSizer1OK->SetDefault();
        GetSizer()->SetSizeHints( this );
        Centre();

        Connect( ID_USE_ABS_PATH, wxEVT_UPDATE_UI,
                 wxUpdateUIEventHandler( DIALOG_EXPORT_3DFILE::OnUpdateUseRelativePath ) );
    }

    ~DIALOG_EXPORT_3DFILE()
    {
        m_unitsOpt = GetUnits();
        m_copy3DFilesOpt = GetCopyFilesOption();
        m_config->Write( OPTKEY_OUTPUT_UNIT, m_unitsOpt );
        m_config->Write( OPTKEY_3DFILES_OPT, m_copy3DFilesOpt );
        m_config->Write( OPTKEY_USE_RELATIVE_PATHS, m_useRelativePathsOpt );
        m_config->Write( OPTKEY_USE_PLAIN_PCB, m_usePlainPCBOpt );
        m_config->Write( OPTKEY_VRML_REF_UNITS, m_VRML_RefUnitChoice->GetSelection() );
        m_config->Write( OPTKEY_VRML_REF_X, m_VRML_Xref->GetValue() );
        m_config->Write( OPTKEY_VRML_REF_Y, m_VRML_Yref->GetValue() );
    };

    void SetSubdir( const wxString & aDir )
    {
        m_SubdirNameCtrl->SetValue( aDir );
    }

    wxString GetSubdir()
    {
        return m_SubdirNameCtrl->GetValue();
    }

    wxFilePickerCtrl* FilePicker()
    {
        return m_filePicker;
    }

    int GetRefUnitsChoice()
    {
        return m_VRML_RefUnitChoice->GetSelection();
    }

    double GetXRef()
    {
        return DoubleValueFromString( UNSCALED_UNITS, m_VRML_Xref->GetValue() );
    }

    double GetYRef()
    {
        return DoubleValueFromString( UNSCALED_UNITS, m_VRML_Yref->GetValue() );
    }

    int GetUnits()
    {
        return m_unitsOpt = m_rbSelectUnits->GetSelection();
    }

    bool GetCopyFilesOption()
    {
        return m_copy3DFilesOpt = m_cbCopyFiles->GetValue();
    }

    bool GetUseRelativePathsOption()
    {
        return m_useRelativePathsOpt = m_cbUseRelativePaths->GetValue();
    }

    bool GetUsePlainPCBOption()
    {
        return m_usePlainPCBOpt = m_cbPlainPCB->GetValue();
    }

    void OnUpdateUseRelativePath( wxUpdateUIEvent& event )
    {
        // Making path relative or absolute has no meaning when VRML files are not copied.
        event.Enable( m_cbCopyFiles->GetValue() );
    }

    virtual void OnFileChanged( wxFileDirPickerEvent& event )
    {
        // Clicking on file picker button changes the focus to the file picker button which
        // is not the behavior we want.  Set the focus back to the OK button so the next enter
        // key press dismisses this dialog with the OK action.
        m_sdbSizer1OK->SetFocus();
    }

};


void PCB_EDIT_FRAME::OnExportVRML( wxCommandEvent& event )
{
    static wxString mruPath;
    static wxString subDirFor3Dshapes;
    wxFileName fn;
    wxString   projectPath;

    if( !wxGetEnv( wxT( "KIPRJMOD" ), &projectPath ) )
        projectPath = wxFileName::GetCwd();

    if( mruPath.IsEmpty() )
        mruPath = projectPath;

    if( subDirFor3Dshapes.IsEmpty() )
    {
        subDirFor3Dshapes = wxT( "shapes3D" );
    }

    // The general VRML scale factor
    // Assuming the VRML default unit is the mm
    // this is the mm to VRML scaling factor for mm, 0.1 inch, and inch
    double scaleList[4] = { 1.0, 0.001, 10.0/25.4, 1.0/25.4 };

    // Build default file name
    fn = GetBoard()->GetFileName();
    fn.SetExt( wxT( "wrl" ) );
    fn.SetPath( mruPath );

    DIALOG_EXPORT_3DFILE dlg( this );
    dlg.FilePicker()->SetPath( fn.GetFullPath() );
    dlg.SetSubdir( subDirFor3Dshapes );

    if( dlg.ShowModal() != wxID_OK )
        return;

    double aXRef = dlg.GetXRef();
    double aYRef = dlg.GetYRef();

    if( dlg.GetRefUnitsChoice() == 1 )
    {
        // selected reference unit is in inches
        aXRef *= 25.4;
        aYRef *= 25.4;
    }

    double scale = scaleList[dlg.GetUnits()];     // final scale export
    bool export3DFiles = dlg.GetCopyFilesOption();
    bool useRelativePaths = dlg.GetUseRelativePathsOption();
    bool usePlainPCB = dlg.GetUsePlainPCBOption();
    wxString fullFilename = dlg.FilePicker()->GetPath();
    wxFileName modelPath = fullFilename;
    wxBusyCursor dummy;

    modelPath.AppendDir( dlg.GetSubdir() );
    subDirFor3Dshapes = dlg.GetSubdir();
    mruPath = dlg.FilePicker()->GetPath();

    wxLogDebug( wxT( "Exporting enabled=%d to %s." ),
                export3DFiles, GetChars( subDirFor3Dshapes ) );

    if( export3DFiles && !modelPath.DirExists() )
    {
        modelPath.Mkdir();
    }

    if( !ExportVRML_File( fullFilename, scale, export3DFiles, useRelativePaths,
                          usePlainPCB, modelPath.GetPath(),
                          aXRef, aYRef ) )
    {
        wxString msg = _( "Unable to create file " ) + fullFilename;
        wxMessageBox( msg );
        return;
    }
}
