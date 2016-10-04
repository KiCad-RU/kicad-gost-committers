/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * https://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef DIALOG_SPICE_MODEL_H
#define DIALOG_SPICE_MODEL_H

#include "dialog_spice_model_base.h"

#include <sim/spice_value.h>
#include <sch_component.h>

#include <wx/valnum.h>

class DIALOG_SPICE_MODEL : public DIALOG_SPICE_MODEL_BASE
{
public:
    DIALOG_SPICE_MODEL( wxWindow* aParent, SCH_COMPONENT& aComponent, SCH_FIELDS& aSchFields );

private:
    /**
     * Parse a string describing a power source, so appropriate settings are checked in the dialog
     * @param aModel contains the string to be parse (e.g. sin(0 1 10k))
     * @return True if the input string was parsed without errors.
     */
    bool parsePowerSource( const wxString& aModel );

    /**
     * Generates a string to describe power source parameters, basing on the current selection.
     * If there are missing fields, it will not modify the target string.
     * @param aTarget is the destination for the generated string.
     * @return True if the string was saved successfully.
     */
    bool generatePowerSource( wxString& aTarget ) const;

    /**
     * Loads a list of components from a file and adds them to a combo box.
     * @param aComboBox is the target combo box
     * @param aFilePath is the file to be processed
     * @param aKeyword is the keyword to select the type of components (e.g. "subckt" or "model")
     */
    void updateFromFile( wxComboBox* aComboBox, const wxString& aFilePath, const wxString& aKeyword );

    /**
     * Returns or creates a field in the edited schematic fields vector.
     * @param aFieldType is an SPICE_FIELD enum value.
     * @return Requested field.
     */
    SCH_FIELD& getField( int aFieldType );

    /**
     * Adds a value to the PWL values list.
     * @param aTime is the time value.
     * @param aValue is the source value at the given time.
     * @return True if request has completed successfully, false if the data is invalid.
     */
    bool addPwlValue( const wxString& aTime, const wxString& aValue );

    virtual bool TransferDataFromWindow() override;
    virtual bool TransferDataToWindow() override;

    // The default dialog Validate() calls the validators of all widgets.
    // This is not what we want; We want only validators of the selected page
    // of the notbooks. So disable the wxDialog::Validate(), and let our
    // TransferDataFromWindow doing the job.
    virtual bool Validate() override
    {
        return true;
    }

    virtual void onInitDlg( wxInitDialogEvent& event ) override
    {
        // Call the default wxDialog handler of a wxInitDialogEvent
        TransferDataToWindow();

        // Now all widgets have the size fixed, call FinishDialogSettings
        FinishDialogSettings();
    }

    // Event handlers
    void onSemiSelectLib( wxCommandEvent& event ) override;
    void onSelectIcLib( wxCommandEvent& event ) override;
    void onPwlAdd( wxCommandEvent& event ) override;
    void onPwlRemove( wxCommandEvent& event ) override;

    ///> Edited component
    SCH_COMPONENT& m_component;

    ///> Fields from the component properties dialog
    SCH_FIELDS& m_fields;

    ///> Temporary field values
    std::map<int, wxString> m_fieldsTmp;

    ///> Column identifiers for PWL power source value list
    long m_pwlTimeCol, m_pwlValueCol;

    SPICE_VALIDATOR m_spiceValidator;
    SPICE_VALIDATOR m_spiceEmptyValidator;
    wxTextValidator m_notEmptyValidator;
};

#endif /* DIALOG_SPICE_MODEL_H */
