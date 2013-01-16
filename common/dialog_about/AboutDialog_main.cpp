/***************************************************************
 * Name:      AboutDialog_main.cpp
 * Purpose:   Code for Application Class
 * Author:    Rafael Sokolowski (rafael.sokolowski@web.de)
 * Created:   2010-08-06
 * Copyright: Rafael Sokolowski ()
 * License:
 **************************************************************/
#include <dialog_about.h>
#include <aboutinfo.h>
#include <wx/aboutdlg.h>
#include <wx/textctrl.h>


/* Used icons:
 *  lang_xx_xpm[];      // Icons of various national flags
 *  show_3d_xpm[];      // 3D icon
 *  edit_module_xpm[];
 *  icon_kicad_xpm[];   // Icon of the application
 */
#include <bitmaps.h>
#include <wxstruct.h>
#include <common.h>
#include <appl_wxstruct.h>
#include <build_version.h>


#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( Contributors )

// Helper functions:
static wxString HtmlHyperlink( const wxString& url, const wxString& description = wxEmptyString );
static wxString HtmlNewline( const unsigned int amount = 1 );


/**
 * Initializes the <code>AboutAppInfo</code> object with applicaion specific information.
 *
 * This the object which holds all information about the application
 */
static void InitKiCadAboutNew( AboutAppInfo& info )
{
    // Set application specific icon
    const wxTopLevelWindow * const tlw = wxDynamicCast(::wxGetApp().GetTopWindow(), wxTopLevelWindow);

    if( tlw )
        info.SetIcon( tlw->GetIcon() );
    else
    {
        wxBitmap    bitmap = KiBitmap( icon_kicad_xpm  );
        wxIcon      icon;

        icon.CopyFromBitmap( bitmap );

        info.SetIcon( icon );
    }

    /* Set title */
    info.SetAppName( wxT( ".: " ) + wxGetApp().GetTitle() + wxT( " :." ) );

    /* Copyright information */
    info.SetCopyright( wxT( "(C) 1992-2012 KiCad Developers Team" ) );

    /* KiCad build version */
    wxString version;
    version << wxT( "Build: " ) << GetBuildVersion();
    info.SetBuildVersion( version );

    /* wxWidgets version */
    wxString libVersion;
    libVersion
        << wxT( "wxWidgets " )
        << wxMAJOR_VERSION << wxT( "." )
        << wxMINOR_VERSION << wxT( "." )
        << wxRELEASE_NUMBER

    /* Unicode or Ansi version */
#if wxUSE_UNICODE
        << wxT( " Unicode " );
#else
        << wxT( " Ansi " );
#endif

    libVersion << wxT( "and boost C++ libraries" );

    libVersion << wxT( "\n" );

    /* Operating System Information */

#if defined __WIN64__
    libVersion << wxT( "on 64 Bits Windows" );

#   elif defined __WINDOWS__
    libVersion << wxT( "on 32 Bits Windows" );

    /* Check for wxMAC */
#   elif defined __WXMAC__
    libVersion << wxT( "on Macintosh" );

    /* Linux 64 bits */
#   elif defined _LP64 && __LINUX__
    libVersion << wxT( "on 64 Bits GNU/Linux" );

    /* Linux 32 bits */
#   elif defined __LINUX__
    libVersion << wxT( "on 32 Bits GNU/Linux" );

    /* OpenBSD */
#   elif defined __OpenBSD__
    libVersion << wxT( "on OpenBSD" );

    /* FreeBSD */
#   elif defined __FreeBSD__
    libVersion << wxT( "on FreeBSD" );

#endif

    info.SetLibVersion( libVersion );


    /* info/description part HTML formatted */

    wxString description;

    /* short description */
    description << wxT( "<p>" );
    description << wxT( "<b><u>" ) << _( "Description" ) << wxT( "</u></b>" ); // bold & underlined font for caption

    description << wxT( "<p>" ) <<
    _(
        "The KiCad EDA Suite is a set of open source applications for the creation of electronic schematics and to design printed circuit boards." )
                << wxT( "</p>" );

    description << wxT( "</p>" );

    /* websites */
    description << wxT( "<p>" );
    description << wxT( "<b><u>" ) << _( "KiCad on the web" ) << wxT( "</u></b>" ); // bold & underlined font for caption

    // bulletet list with some http links
    description << wxT( "<ul>" );
    description << wxT( "<li>" ) << HtmlHyperlink( wxT(
                                                      "http://iut-tice.ujf-grenoble.fr/kicad" ),
                                                  _( "The original site of the initiator of Kicad" ) )
                << wxT( "</li>" );
    description << wxT( "<li>" ) <<
    HtmlHyperlink( wxT( "https://launchpad.net/kicad" ), _( "Project on Launchpad" ) ) << wxT(
        "</li>" );
    description << wxT( "<li>" ) <<
    HtmlHyperlink( wxT( "http://www.kicad-pcb.org" ),
                  _( "The new KiCad site" ) ) << wxT( "</li>" );
    description << wxT( "<li>" ) <<
    HtmlHyperlink( wxT( "http://www.kicadlib.org" ),
                  _( "Repository with additional component libraries" ) ) << wxT( "</li>" );
    description << wxT( "</ul>" );

    description << wxT( "</p>" );

    description << wxT( "<p>" );
    description << wxT( "<b><u>" ) << _( "Contribute to KiCad" ) << wxT( "</u></b>" ); // bold & underlined font caption

    // bulletet list with some http links
    description << wxT( "<ul>" );
    description << wxT( "<li>" ) <<
    HtmlHyperlink( wxT( "https://bugs.launchpad.net/kicad" ),
                  _( "Report bugs if you found any" ) ) << wxT( "</li>" );
    description << wxT( "<li>" ) << HtmlHyperlink( wxT(
                                                      "https://blueprints.launchpad.net/kicad" ),
                                                  _( "File an idea for improvement" ) ) << wxT(
        "</li>" );
    description << wxT( "<li>" ) <<
    HtmlHyperlink( wxT( "http://www.kicadlib.org/Kicad_related_links.html" ),
                  _( "KiCad links to user groups, tutorials and much more" ) ) << wxT( "</li>" );
    description << wxT( "</ul>" );

    description << wxT( "</p>" );

    info.SetDescription( description );


    /* License information also HTML formatted */
    wxString license;
    license
        << wxT( "<div align='center'>" )
        << HtmlNewline( 4 )
        << _( "The complete KiCad EDA Suite is released under the" ) << HtmlNewline( 2 )
        << HtmlHyperlink( wxT( "http://www.gnu.org/licenses" ),
                         _( "GNU General Public License (GPL) version 2" ) )
        << wxT( "</div>" );

    info.SetLicense( license );


    /* A contributor consists of the following information:
     * Mandatory:
     * - Name
     * - EMail address
     * Optional:
     * - Category
     * - Category specific icon
     *
     * All contributors of the same category will be enumerated under this category
     * which should be represented by the same icon.
     */

    /* The developers */
    info.AddDeveloper( new Contributor( wxT( "Jean-Pierre Charras" ),
                                        wxT( "jp.charras@wanadoo.fr" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Dick Hollenbeck" ), wxT( "dick@softplc.com" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Frank Bennett" ), wxT( "bennett78@lpbroadband.net" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Hauptmech" ), wxT( "hauptmech@gmail.com" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Jerry Jacobs" ),
                                        wxT( "xor.gate.engineering@gmail.com" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Jonas Diemer" ), wxT( "diemer@gmx.de" ) ) );
    info.AddDeveloper( new Contributor( wxT( "KBool Library" ),
                                        wxT( "http://boolean.klaasholwerda.nl/bool.html" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Lorenzo Marcantonio" ), wxT( "lomarcan@tin.it" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Marco Serantoni" ),
                                        wxT( "marco.serantoni@gmail.com" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Marco Mattila" ), wxT( "marcom99@gmail.com" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Miguel Angel Ajo Pelayo" ),
                                        wxT( "miguelangel@nbee.es" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Rafael Sokolowski" ),
                                        wxT( "rafael.sokolowski@web.de" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Rok Markovic" ), wxT( "rok@kanardia.eu" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Tim Hanson" ), wxT( "sideskate@gmail.com" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Vesa Solonen" ), wxT( "vesa.solonen@hut.fi" ) ) );
    info.AddDeveloper( new Contributor( wxT( "Wayne Stambaugh" ),
                                        wxT( "stambaughw@verizon.net" ) ) );

    /* The document writers */
    info.AddDocWriter( new Contributor( wxT( "Jean-Pierre Charras" ),
                                        wxT( "jp.charras@wanadoo.fr" ) ) );
    info.AddDocWriter( new Contributor( wxT( "Igor Plyatov" ),
                                        wxT( "plyatov@gmail.com" ) ) );
    info.AddDocWriter( new Contributor( wxT( "Fabrizio Tappero" ),
                                        wxT( "fabrizio.tappero@gmail.com" ) ) );

    /* The translators
     * As category the language to which the translation was done is used
     * and as icon the national flag of the corresponding country.
     */
    info.AddTranslator( new Contributor( wxT( "Martin Kratoška" ), wxT( "martin@ok1rr.com" ),
                                         wxT( "Czech (CZ)" ), KiBitmapNew( lang_cs_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Jerry Jacobs" ),
                                         wxT( "xor.gate.engineering@gmail.com" ), wxT( "Dutch (NL)" ),
                                         KiBitmapNew( lang_nl_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Vesa Solonen" ), wxT( "vesa.solonen@hut.fi" ),
                                         wxT( "Finnish (FI)" ), KiBitmapNew( lang_fi_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Jean-Pierre Charras" ),
                                         wxT( "jp.charras@wanadoo.fr" ),
                                         wxT( "French (FR)" ), KiBitmapNew( lang_fr_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Mateusz Skowroński" ), wxT( "skowri@gmail.com" ),
                                         wxT( "Polish (PL)" ), KiBitmapNew( lang_pl_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Renie Marquet" ), wxT( "reniemarquet@uol.com.br" ),
                                         wxT( "Portuguese (PT)" ), KiBitmapNew( lang_pt_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Igor Plyatov" ), wxT( "plyatov@gmail.com" ),
                                         wxT( "Russian (RU)" ), KiBitmapNew( lang_ru_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Andrey Fedorushkov" ), wxT( "andrf@mail.ru" ),
                                         wxT( "Russian (RU)" ), KiBitmapNew( lang_ru_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Pedro Martin del Valle" ), wxT( "pkicad@yahoo.es" ),
                                         wxT( "Spanish (ES)" ), KiBitmapNew( lang_es_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Iñigo Zuluaga" ), wxT( "inigo_zuluaga@yahoo.es" ),
                                         wxT( "Spanish (ES)" ), KiBitmapNew( lang_es_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Rafael Sokolowski" ),
                                         wxT( "rafael.sokolowski@web.de" ), wxT( "German (DE)" ),
                                         KiBitmapNew( lang_de_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Kenta Yonekura" ),
                                         wxT( "midpika@hotmail.com" ), wxT( "Japanese (JA)" ),
                                         KiBitmapNew( lang_jp_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Manolis Stefanis, Athanasios Vlastos and Milonas Kostas" ),
                                         wxT( "milonas.ko@gmail.com" ), wxT( "Greek (el_GR)" ),
                                         KiBitmapNew( lang_gr_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Massimo Cioce" ),
                                         wxT( "ciocemax@alice.it" ), wxT( "Italian (IT)" ),
                                         KiBitmapNew( lang_it_xpm ) ) );
    info.AddTranslator( new Contributor( wxT( "Evgeniy Ivanov" ),
                                         wxT( "evgeniy_p_ivanov@yahoo.ca" ), wxT( "Bulgarian (BG)" ),
                                         KiBitmapNew( lang_bg_xpm ) ) );

    // TODO: are these all russian translators,
    // placed them here now,
    // or else align them below other language maintainer with mail adress
    info.AddTranslator( new Contributor( wxT( "Remy Halvick" ), wxEmptyString, wxT( "Others" ) ) );
    info.AddTranslator( new Contributor( wxT( "David Briscoe" ), wxEmptyString, wxT( "Others" ) ) );
    info.AddTranslator( new Contributor( wxT( "Dominique Laigle" ), wxEmptyString, wxT( "Others" ) ) );
    info.AddTranslator( new Contributor( wxT( "Paul Burke" ), wxEmptyString, wxT( "Others" ) ) );

    /* Programm credits for icons */
    info.AddArtist( new Contributor( wxT( "Iñigo Zuluagaz" ), wxT( "inigo_zuluaga@yahoo.es" ),
                                     wxT( "Icons by" ), KiBitmapNew( edit_module_xpm ) ) );
    info.AddArtist( new Contributor( wxT( "Fabrizio Tappero" ), wxT( "fabrizio.tappero@gmail.com" ),
                                     wxT( "New icons by" ), KiBitmapNew( edit_module_xpm ) ) );
    info.AddArtist( new Contributor( wxT( "Renie Marquet" ), wxT( "reniemarquet@uol.com.br" ),
                                     wxT( "3D modules by" ), KiBitmapNew( three_d_xpm ) ) );
    info.AddArtist( new Contributor( wxT( "Christophe Boschat" ), wxT( "nox454@hotmail.fr" ),
                                     wxT( "3D modules by" ), KiBitmapNew( three_d_xpm ) ) );
}


bool ShowAboutDialog( wxWindow* parent )
{
    AboutAppInfo info;

    InitKiCadAboutNew( info );

    dialog_about* dlg = new dialog_about( parent, info );
    dlg->SetIcon( info.GetIcon() );
    dlg->Show();

    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// Helper functions
///////////////////////////////////////////////////////////////////////////////

/**
 * Wraps the given url with a HTML anchor tag containing a hyperlink text reference
 * to form a HTML hyperlink.
 *
 * @param url the url that will be embedded in an anchor tag containing a hyperlink reference
 * @param description the optional describing text that will be represented as a hyperlink.
 *  If not specified the url will be used as hyperlink.
 * @return a HTML conform hyperlink like <a href='url'>description</a>
 */
static wxString HtmlHyperlink( const wxString& url, const wxString& description )
{
    wxString hyperlink = wxEmptyString;

    if( description.IsEmpty() )
        hyperlink << wxT( "<a href='" ) << url << wxT( "'>" ) << url << wxT( "</a>" );
    else
        hyperlink << wxT( "<a href='" ) << url << wxT( "'>" ) << description << wxT( "</a>" );

    return hyperlink;
}


/**
 * Creates a HTML newline character sequence.
 *
 * @param amount - the amount of HTML newline tags to concatenate, default is to return just
 *                  one <br> tag
 * @return the concatenated amount of HTML newline tag(s) <br>
 */
static wxString HtmlNewline( const unsigned int amount )
{
    wxString newlineTags = wxEmptyString;

    for( size_t i = 0; i<amount; ++i )
        newlineTags << wxT( "<br>" );

    return newlineTags;
}
