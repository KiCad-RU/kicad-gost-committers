/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2016 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file dialog_eeschema_options.h
 *
 * Subclass of DIALOG_EESCHEMA_OPTIONS_BASE, which is generated by wxFormBuilder.
 */

#ifndef __dialog_eeschema_options__
#define __dialog_eeschema_options__

#include <dialog_eeschema_options_base.h>
#include <template_fieldnames.h>

class WIDGET_EESCHEMA_COLOR_CONFIG;
class WIDGET_HOTKEY_LIST;
class SCH_EDIT_FRAME;

class DIALOG_EESCHEMA_OPTIONS : public DIALOG_EESCHEMA_OPTIONS_BASE
{
protected:
    WIDGET_HOTKEY_LIST*             m_hotkeyListCtrl;
    WIDGET_EESCHEMA_COLOR_CONFIG*   m_colorConfigCtrl;

    /** @brief The template fieldnames for this dialog */
    TEMPLATE_FIELDNAMES templateFields;

    /**
     * Function OnAddButtonClick
     * Process the wxWidgets @a event produced when the user presses the Add buton for the
     * template fieldnames control
     *
     * @param event The wxWidgets produced event information
     *
     * Adds a new template fieldname (with default values) to the template fieldnames data
     */
    void OnAddButtonClick( wxCommandEvent& event );

    /**
     * Function OnDeleteButtonClick
     * Process the wxWidgets @a event produced when the user presses the Delete button for the
     * template fieldnames control
     *
     * @param event The wxWidgets produced event information
     *
     * Deletes the selected template fieldname from the template fieldnames data
     */
    void OnDeleteButtonClick( wxCommandEvent& event );

    /**
     * Function TransferDataToWindow
     * Transfer data into the GUI.
     */
    bool TransferDataToWindow();

    /**
     * Function TransferDataFromWindow
     * Transfer data out of the GUI.
     */
    bool TransferDataFromWindow();

public:
    /**
     * Public constructor
     *
     * @param parent The dialog's parent
     */
    DIALOG_EESCHEMA_OPTIONS( SCH_EDIT_FRAME* parent );

    virtual SCH_EDIT_FRAME* GetParent();

    /**
     * Function GetUnitsSelection
     * Returns the currently selected grid size in the dialog
     */
    int GetUnitsSelection( void ) { return m_choiceUnits->GetSelection(); }

    /**
     * Function SetUnits
     * Set the unit options
     *
     * @param units The array of strings representing the unit options
     * @param select The unit to select from the unit options
     *
     * Appends the @a units options to the list of unit options and selects the @a aSelect option
     */
    void SetUnits( const wxArrayString& units, int aSelect = 0 );

    /**
     * Function GetGridSelection
     * Returns the curent grid size selected in the dialog
     */
    int GetGridSelection( void ) { return m_choiceGridSize->GetSelection(); }

    /**
     * Function SetGridSizes
     * Sets the available grid size choices @a aGridSizes and selectd the current option @a aGridId
     *
     * @param aGridSizes The grid sizes that are able to be chosen from
     * @param aGridId The grid size to select from the grid size options
     */
    void SetGridSizes( const GRIDS& aGridSizes, int aGridId );

    /**
     * Function GetBusWidth
     * Get the current bus width setting from the dialog
     */
    int GetBusWidth( void ) { return m_spinBusWidth->GetValue(); }

    /**
     * Function SetBusWidth
     * Sets the bus width setting in the dialog
     *
     * @param aWidth The bus width to set the dialog edit spinbox with
     */
    void SetBusWidth( int aWidth ) { m_spinBusWidth->SetValue( aWidth ); }

    /**
     * Function SetLineWidth
     * Sets the current LineWidth value in the dialog
     * @param aWidth The line width to set in the dialog
     */
    void SetLineWidth( int aWidth ) { m_spinLineWidth->SetValue( aWidth ); }

    /**
     * Function GetLineWidth
     * Returns the current LineWidth value from the dialog
     */
    int GetLineWidth( void ) { return m_spinLineWidth->GetValue(); }

    /**
     * Function SetTextSize
     * Sets the current default TextSize value in the dialog
     * @param text_size The text size to set in the dialog
     */
    void SetTextSize( int text_size ) { m_spinTextSize->SetValue( text_size ); }

    /**
     * Function GetTextSize
     * Returns the current default TextSize value from the dialog
     */
    int GetTextSize( void ) { return m_spinTextSize->GetValue(); }

    /**
     * Function SetRepeatHorizontal
     * Sets the current RepeatHorizontal displacement value in the dialog
     * @param displacement The displacement to set in the dialog
     */
    void SetRepeatHorizontal( int displacement )
    {
        m_spinRepeatHorizontal->SetValue( displacement );
    }

    /**
     * Function GetRepeatHorizontal
     * Returns the current RepeatHorizontal displacement value from the dialog
     */
    int GetRepeatHorizontal( void ) { return m_spinRepeatHorizontal->GetValue(); }

    /**
     * Function SetRepeatVertical
     * Sets the current RepeatVertical displacement value in the dialog
     * @param displacement The displacement to set in the dialog
     */
    void SetRepeatVertical( int displacement ) { m_spinRepeatVertical->SetValue( displacement ); }

    /**
     * Function GetRepeatVertical
     * Returns the current RepeatVertical displacement value from the dialog
     */
    int GetRepeatVertical( void ) { return m_spinRepeatVertical->GetValue(); }

    /**
     * Function SetRepeatLabel
     * Sets the current RepeatLabel increment value in the dialog
     * @param increment The increment to set in the dialog
     */
    void SetRepeatLabel( int increment ) { m_spinRepeatLabel->SetValue( increment ); }

    /**
     * Function GetRepeatLabel
     * Returns the current RepeatLabel increment value from the dialog
     */
    int GetRepeatLabel( void ) { return m_spinRepeatLabel->GetValue(); }

    /**
     * Function SetAutoSaveInterval
     * Sets the current AutoSaveInterval value in the dialog
     * @param aInterval The interval to set in the dialog
     */
    void SetAutoSaveInterval( int aInterval ) { m_spinAutoSaveInterval->SetValue( aInterval ); }

    /**
     * Function GetAutoSaveInterval
     * Returns the current AutoSaveInterval value from the dialog
     */
    int GetAutoSaveInterval() const { return m_spinAutoSaveInterval->GetValue(); }

    /**
     * Function SetRefIdSeparator
     * Sets the current RefIdSeparator value in the dialog
     * @param aSep The seperator to use between the reference and the part ID
     * @param aFirstId The first part ID, currently either 'A' or '1'
     */
    void SetRefIdSeparator( wxChar aSep, wxChar aFirstId);

    /**
     * Function GetRefIdSeparator
     * Returns the current RefIdSeparator value from the dialog
     * @param aSep The OUTPUT seperator value
     * @param aFirstId The OUTPUT reference first ID
     */
    void GetRefIdSeparator( int& aSep, int& aFirstId);

    /**
     * Function SetShowGrid
     * Sets the current ShowGrid value in the dialog
     * @param show The ShowGrid value to set in the dialog
     */
    void SetShowGrid( bool show ) { m_checkShowGrid->SetValue( show ); }

    /**
     * Function GetShowGrid
     * Returns the current ShowGrid value from the dialog
     */
    bool GetShowGrid( void ) { return m_checkShowGrid->GetValue(); }

    /**
     * Function SetShowHiddenPins
     * Sets the current ShowHiddenPins value in the dialog
     * @param show The ShowHiddenPins value to set in the dialog
     */
    void SetShowHiddenPins( bool show ) { m_checkShowHiddenPins->SetValue( show ); }

    /**
     * Function GetShowHiddenPins
     * Returns the current ShowHiddenPins value from the dialog
     */
    bool GetShowHiddenPins( void ) { return m_checkShowHiddenPins->GetValue(); }

    /**
     * Function SetEnableZoomNoCenter
     * Sets the current ZoomNoCenter value in the dialog
     * @param enable The ZoomNoCenter value to set in the dialog
     */
    void SetEnableZoomNoCenter( bool enable )
    {
        m_checkEnableZoomCenter->SetValue( !enable );
    }

    /**
     * Function GetEnableZoomNoCenter
     * Returns the current ZoomNoCenter value from the dialog
     */
    bool GetEnableZoomNoCenter( void )
    {
        return !m_checkEnableZoomCenter->GetValue();
    }

    /**
     * Function SetEnableMiddleButtonPan
     * Sets the current MiddleButtonPan value in the dialog
     *
     * @param enable The boolean value to set the MiddleButtonPan value in the dialog
     */
    void SetEnableMiddleButtonPan( bool enable )
    {
        m_checkEnableMiddleButtonPan->SetValue( enable );
        m_checkMiddleButtonPanLimited->Enable( enable );
    }

    /**
     * Function GetEnableMiddleButtonPan
     * Returns the current MiddleButtonPan setting from the dialog
     */
    bool GetEnableMiddleButtonPan( void )
    {
        return m_checkEnableMiddleButtonPan->GetValue();
    }

    /**
     * Function SetMiddleButtonPanLimited
     * Sets the MiddleButtonPanLimited value in the dialog
     *
     * @param enable The boolean value to set the MiddleButtonPanLimted value in the dialog
     */
    void SetMiddleButtonPanLimited( bool enable )
    {
        m_checkMiddleButtonPanLimited->SetValue( enable );
    }

    /**
     * Function GetMiddleButtonPanLimited
     * Returns the MiddleButtonPanLimited setting from the dialog
     */
    bool GetMiddleButtonPanLimited( void )
    {
        return m_checkMiddleButtonPanLimited->GetValue();
    }

    /**
     * Function SetEnableMousewheelPan
     * Sets the MousewheelPan setting in the dialog
     *
     * @param enable The boolean value to set the AutoPan value in the dialog
     */
    void SetEnableMousewheelPan( bool enable ) { m_checkEnableMousewheelPan->SetValue( enable ); }

    /**
     * Function GetEnableMousewheelPan
     * Return the MousewheelPan setting from the dialog
     */
    bool GetEnableMousewheelPan( void ) { return m_checkEnableMousewheelPan->GetValue(); }

    /**
     * Function SetEnableAutoPan
     * Sets the AutoPan setting in the dialog
     *
     * @param enable The boolean value to set the AutoPan value in the dialog
     */
    void SetEnableAutoPan( bool enable ) { m_checkAutoPan->SetValue( enable ); }

    /**
     * Function GetEnableAutoPan
     * Return the AutoPan setting from the dialog
     */
    bool GetEnableAutoPan( void ) { return m_checkAutoPan->GetValue(); }

    /**
     * Function SetEnableHVBusOrientation
     * Set the HVBusOrientation setting in the dialog
     *
     * @param enable The boolean value to set the HVBusOrientation value in the dialog
     */
    void SetEnableHVBusOrientation( bool enable ) { m_checkHVOrientation->SetValue( enable ); }

    /**
     * Function GetEnableHVBusOrientation
     * Get the HVBusOrientation setting from the dialog
     */
    bool GetEnableHVBusOrientation( void ) { return m_checkHVOrientation->GetValue(); }

    /**
     * Function
     * Set the ShowPageLimits setting in the dialog
     */
    void SetShowPageLimits( bool show ) { m_checkPageLimits->SetValue( show ); }

    /**
     * Function
     * Return the current ShowPageLimits setting from the dialog
     */
    bool GetShowPageLimits( void ) { return m_checkPageLimits->GetValue(); }

    /**
     * Function
     * Set the AutoplaceFields setting in the dialog
     */
    void SetAutoplaceFields( bool enable ) { m_checkAutoplaceFields->SetValue( enable ); }

    /**
     * Function
     * Return the current AutoplaceFields setting from the dialog
     */
    bool GetAutoplaceFields() { return m_checkAutoplaceFields->GetValue(); }

    /**
     * Function
     * Set the AutoplaceJustify setting in the dialog
     */
    void SetAutoplaceJustify( bool enable ) { m_checkAutoplaceJustify->SetValue( enable ); }

    /**
     * Function
     * Return the current AutoplaceJustify setting from the dialog
     */
    bool GetAutoplaceJustify() { return m_checkAutoplaceJustify->GetValue(); }

    /**
     * Function
     * Set the AutoplaceAlign setting in the dialog
     */
    void SetAutoplaceAlign( bool enable ) { m_checkAutoplaceAlign->SetValue( enable ); }

    /**
     * Function
     * Return the current AutoplaceAlign setting from the dialog
     */
    bool GetAutoplaceAlign() { return m_checkAutoplaceAlign->GetValue(); }

    /**
     * Function SetTemplateFields
     * Set the template field data in the dialog
     *
     * @param aFields The template fieldnames that the dialog should start with before any editing
     */
    void SetTemplateFields( const TEMPLATE_FIELDNAMES& aFields );

    /**
     * Function GetTemplateFields
     * Get the dialog's template field data
     *
     */
    TEMPLATE_FIELDNAMES GetTemplateFields( void );

private:
    void OnMiddleBtnPanEnbl( wxCommandEvent& event )
    {
        m_checkMiddleButtonPanLimited->Enable( GetEnableMiddleButtonPan() );
    }
};

#endif // __dialog_eeschema_options__
