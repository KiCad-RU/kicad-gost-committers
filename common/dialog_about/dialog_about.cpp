/***************************************************************
 * Name:      dialog_about.cpp
 * Purpose:   Code for Application Frame
 * Author:    Rafael Sokolowski (rafael.sokolowski@web.de)
 * Created:   2010-08-06
 * Copyright: Rafael Sokolowski ()
 * License:
 **************************************************************/
#include <dialog_about.h>

///////////////////////////////////////////////////////////////////////////////
/// Class dialog_about methods
///////////////////////////////////////////////////////////////////////////////

dialog_about::dialog_about(wxWindow *parent, AboutAppInfo& appInfo)
    : dialog_about_base(parent), info(appInfo)
{
    picInformation = KiBitmap( info_xpm );
    picDevelopers  = KiBitmap( preference_xpm );
    picDocWriters  = KiBitmap( editor_xpm );
    picArtists     = KiBitmap( palette_xpm );
    picTranslators = KiBitmap( language_xpm );
    picLicense     = KiBitmap( tools_xpm );

    m_bitmapApp->SetBitmap( info.GetIcon() );

    m_staticTextAppTitle->SetLabel( info.GetAppName() );
    m_staticTextCopyright->SetLabel( info.GetCopyright() );
    m_staticTextBuildVersion->SetLabel( info.GetBuildVersion() );
    m_staticTextLibVersion->SetLabel( info.GetLibVersion() );

    /* Affects m_titlepanel the parent of some wxStaticText.
     * Changing the text afterwards makes it under Windows necessary to call 'Layout()'
     * so that the new text gets properly layout.
     */
/*    m_staticTextCopyright->GetParent()->Layout();
    m_staticTextBuildVersion->GetParent()->Layout();
    m_staticTextLibVersion->GetParent()->Layout();
*/
    DeleteNotebooks();
    CreateNotebooks();
    GetSizer()->SetSizeHints(this);
    m_auiNotebook->Update();
    SetFocus();
    Centre();
}

dialog_about::~dialog_about()
{
}

wxFlexGridSizer* dialog_about::CreateFlexGridSizer()
{
    // three colums with vertical and horizontal extra space of two pixels
    wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer( 3, 2, 2 );
    fgSizer1->SetFlexibleDirection( wxHORIZONTAL );
    fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    return fgSizer1;
}

void dialog_about::DeleteNotebooks()
{
    for( size_t i=0; i<m_auiNotebook->GetPageCount(); ++i )
        m_auiNotebook->DeletePage(i);
}

void dialog_about::CreateNotebooks()
{
    CreateNotebookHtmlPage( m_auiNotebook, _("Information"), picInformation, info.GetDescription() );

    CreateNotebookPage( m_auiNotebook, _("Developers") , picDevelopers, info.GetDevelopers() );
    CreateNotebookPage( m_auiNotebook, _("Doc Writers"), picDocWriters, info.GetDocWriters() );

    CreateNotebookPageByCategory( m_auiNotebook, _("Artists")    , picArtists,     info.GetArtists() );
    CreateNotebookPageByCategory( m_auiNotebook, _("Translators"), picTranslators, info.GetTranslators() );

    CreateNotebookHtmlPage( m_auiNotebook, _("License"), picLicense, info.GetLicense() );
}

void dialog_about::CreateNotebookPage(wxAuiNotebook* parent, const wxString& caption, const wxBitmap& icon, const Contributors& contributors)
{
    wxBoxSizer* bSizer = new wxBoxSizer( wxHORIZONTAL );

    wxScrolledWindow* m_scrolledWindow1 = new wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
    m_scrolledWindow1->SetScrollRate( 5, 5 );

    /* Panel for additional space at the left,
     * but can also be used to show an additional bitmap.
     */
    wxPanel* panel1 = new wxPanel(m_scrolledWindow1);

    wxFlexGridSizer* fgSizer1 = CreateFlexGridSizer();

    for ( size_t i=0; i<contributors.GetCount(); ++i)
    {
        Contributor* contributor = &contributors.Item(i);

        // Icon at first column
        wxStaticBitmap* m_bitmap1 = CreateStaticBitmap( m_scrolledWindow1, contributor->GetIcon() );
        fgSizer1->Add( m_bitmap1, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );

        // Name of contributor at second column
        if ( contributor->GetName() != wxEmptyString )
        {
            wxStaticText* m_staticText1 = new wxStaticText( m_scrolledWindow1, wxID_ANY, contributor->GetName(), wxDefaultPosition, wxDefaultSize, 0 );
            m_staticText1->Wrap( -1 );
            fgSizer1->Add( m_staticText1, 0, wxALIGN_LEFT|wxBOTTOM, 2 );
        }
        else {
            fgSizer1->AddSpacer(5);
        }

        // Email address of contributor at third column
        if ( contributor->GetEMail() != wxEmptyString )
        {
            wxHyperlinkCtrl* hyperlink = CreateHyperlink( m_scrolledWindow1, contributor->GetEMail() );
            fgSizer1->Add( hyperlink, 0, wxALIGN_LEFT|wxBOTTOM, 2 );
        }
        else {
            fgSizer1->AddSpacer(5);
        }
    }

    bSizer->Add( panel1, 1, wxEXPAND|wxALL, 10 );
    bSizer->Add( fgSizer1, 7, wxEXPAND|wxALL, 10 ); // adjust width of panel with first int value
    m_scrolledWindow1->SetSizer( bSizer );
    m_scrolledWindow1->Layout();
    bSizer->Fit( m_scrolledWindow1 );

    parent->AddPage( m_scrolledWindow1, caption, false, icon );
}

void dialog_about::CreateNotebookPageByCategory(wxAuiNotebook* parent, const wxString& caption, const wxBitmap& icon, const Contributors& contributors)
{
    wxBoxSizer* bSizer = new wxBoxSizer( wxHORIZONTAL );

    wxScrolledWindow* m_scrolledWindow1 = new wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
    m_scrolledWindow1->SetScrollRate( 5, 5 );

    /* Panel for additional space at the left,
     * but can also be used to show an additional bitmap.
     */
    wxPanel* panel1 = new wxPanel(m_scrolledWindow1);

    wxFlexGridSizer* fgSizer1 = CreateFlexGridSizer();

    for ( size_t i=0; i<contributors.GetCount(); ++i)
    {
        Contributor* contributor = &contributors.Item(i);

        wxBitmap* icon = contributor->GetIcon();
        wxString category = contributor->GetCategory();

        /* to construct the next row we expect to have
         * a category and a contributor that was not considered up to now
         */
        if ( ( category != wxEmptyString ) && !( contributor->IsChecked() ) )
        {
            // Icon at first column
            wxStaticBitmap* m_bitmap1 = CreateStaticBitmap( m_scrolledWindow1, icon );
            fgSizer1->Add( m_bitmap1, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );

            // Category name at second column
            wxStaticText* m_staticText1 = new wxStaticText( m_scrolledWindow1, wxID_ANY, contributor->GetCategory() + wxT(":"), wxDefaultPosition, wxDefaultSize, 0 );
            m_staticText1->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) ); // bold font
            m_staticText1->Wrap( -1 );
            fgSizer1->Add( m_staticText1, 0, wxALIGN_LEFT|wxBOTTOM, 2 );

            // Nothing at third column
            fgSizer1->AddSpacer(5);

            // Now, all contributors of the same category will follow
            for ( size_t j=0; j<contributors.GetCount(); ++j )
            {
                Contributor* contributor = &contributors.Item(j);

                if ( contributor->GetCategory() == category )
                {
                    // First column is empty
                    fgSizer1->AddSpacer(5);

                    // Name of contributor at second column
                    wxStaticText* m_staticText2 = new wxStaticText( m_scrolledWindow1, wxID_ANY, wxT(" • ") + contributor->GetName(), wxDefaultPosition, wxDefaultSize, 0 );
                    m_staticText1->Wrap( -1 );
                    fgSizer1->Add( m_staticText2, 0, wxALIGN_LEFT|wxBOTTOM, 2 );

                    // Email address of contributor at third column
                    if ( contributor->GetEMail() != wxEmptyString )
                    {
                        wxHyperlinkCtrl* hyperlink = CreateHyperlink( m_scrolledWindow1, contributor->GetEMail() );
                        fgSizer1->Add( hyperlink, 0, wxALIGN_LEFT|wxBOTTOM, 2 );
                    }
                    else {
                        fgSizer1->AddSpacer(5);
                    }

                    /* this contributor was added to the gui,
                     * thus can be ignored next time
                     */
                    contributor->SetChecked( true );
                }
            }
        }
        else {
            continue;
        }
    }

    /* Now, lets list the remaining contributors that have not been considered
     * because they were not assigned to any category.
     */
    for ( size_t k=0; k<contributors.GetCount(); ++k )
    {
        Contributor* contributor = &contributors.Item(k);

        if ( contributor->IsChecked() )
            continue;

        // Icon at first column
        wxStaticBitmap* m_bitmap1 = CreateStaticBitmap( m_scrolledWindow1, contributor->GetIcon() );
        fgSizer1->Add( m_bitmap1, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );

        // Name of contributor at second column
        if ( contributor->GetName() != wxEmptyString )
        {
            wxStaticText* m_staticText1 = new wxStaticText( m_scrolledWindow1, wxID_ANY, contributor->GetName(), wxDefaultPosition, wxDefaultSize, 0 );
            m_staticText1->Wrap( -1 );
            fgSizer1->Add( m_staticText1, 0, wxALIGN_LEFT|wxBOTTOM, 2 );
        }
        else {
            fgSizer1->AddSpacer(5);
        }

        // Email address of contributor at third column
        if ( contributor->GetEMail() != wxEmptyString )
        {
            wxHyperlinkCtrl* hyperlink = CreateHyperlink( m_scrolledWindow1, contributor->GetEMail() );
            fgSizer1->Add( hyperlink, 0, wxALIGN_LEFT|wxBOTTOM, 2 );
        }
        else {
            fgSizer1->AddSpacer(5);
        }
    }

    bSizer->Add( panel1, 1, wxEXPAND|wxALL, 10 );
    bSizer->Add( fgSizer1, 7, wxEXPAND|wxALL, 10 ); // adjust width of panel with first int value
    m_scrolledWindow1->SetSizer( bSizer );
    m_scrolledWindow1->Layout();
    bSizer->Fit( m_scrolledWindow1 );

    parent->AddPage( m_scrolledWindow1, caption, false, icon );
}

void dialog_about::CreateNotebookHtmlPage(wxAuiNotebook* parent, const wxString& caption, const wxBitmap& icon, const wxString& html)
{
    wxPanel* panel = new wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

    wxBoxSizer* bSizer = new wxBoxSizer( wxVERTICAL );

    wxString htmlPage = wxEmptyString, htmlContent = html;

    // to have a unique look background color for HTML pages is set to the default as it is used for all the other widgets
    wxString htmlColor = ( this->GetBackgroundColour() ).GetAsString( wxC2S_HTML_SYNTAX );

    // beginning of html structure
    htmlPage.Append( wxT("<html><body bgcolor='") + htmlColor + wxT("'>") );

    htmlPage.Append( htmlContent );

    // end of html structure indicated by closing tags
    htmlPage.Append( wxT("</body></html>") );

    // the html page is going to be created with previously created html content
    wxHtmlWindow* htmlWindow = new wxHtmlWindow( panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO|wxHW_NO_SELECTION );

    // HTML font set to font properties as they are used for widgets to have an unique look under different platforms with HTML
    wxFont font = this->GetFont();
    htmlWindow->SetStandardFonts( font.GetPointSize(), font.GetFaceName(), font.GetFaceName() );
    htmlWindow->SetPage( htmlPage );

    // the HTML window shall not be used to open external links, thus this task is delegated to users default browser
    htmlWindow->Connect( wxEVT_COMMAND_HTML_LINK_CLICKED, wxHtmlLinkEventHandler( dialog_about::OnHtmlLinkClicked ), NULL, this );

    // no additional space around the html window as it is also the case by the other notebook pages
    bSizer->Add( htmlWindow, 1, wxALL|wxEXPAND, 0 );
    panel->SetSizer( bSizer );
    panel->Layout();
    bSizer->Fit( panel );

    parent->AddPage( panel, caption, false, icon );
}

wxHyperlinkCtrl* dialog_about::CreateHyperlink(wxScrolledWindow* parent, const wxString& email)
{
    wxHyperlinkCtrl* hyperlink = new wxHyperlinkCtrl(
                                        parent, wxID_ANY,
                                        wxT("<") + email + wxT(">"), /* the label */
                                        wxT("mailto:") + email
                                        + wxT("?subject=KiCad - ")
                                        + info.GetBuildVersion()
                                        + wxT( " ,  ") + info.GetLibVersion()
                                        ); /* the url */

    return hyperlink;
}

wxStaticBitmap* dialog_about::CreateStaticBitmap(wxScrolledWindow* parent, wxBitmap* icon)
{
    wxStaticBitmap* bitmap = new wxStaticBitmap( parent, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );

    if( icon )
    {
        bitmap->SetBitmap( *icon );
    }
    else
    {
        bitmap->SetBitmap( KiBitmap( right_xpm ) );
    }
    return bitmap;
}

///////////////////////////////////////////////////////////////////////////////
/// Event handlers
///////////////////////////////////////////////////////////////////////////////

void dialog_about::OnClose(wxCloseEvent &event)
{
    Destroy();
}

void dialog_about::OnOkClick(wxCommandEvent &event)
{
    Destroy();
}

void dialog_about::OnHtmlLinkClicked( wxHtmlLinkEvent& event )
{
    ::wxLaunchDefaultBrowser( event.GetLinkInfo().GetHref() );
}
