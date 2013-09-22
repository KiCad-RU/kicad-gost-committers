/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2013 KiCad Developers, see CHANGELOG.TXT for contributors.
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


/*
    This is a pcbnew PLUGIN which supports some of the PLUGIN::Footprint*() functions
    in the PLUGIN interface, and could do so by utilizing the version 3 github.com
    API documented here:

        http://developer.github.com
        https://help.github.com/articles/creating-an-access-token-for-command-line-use

    but it does not.  Rather it simply reads in a zip file of the repo and unzips it
    from RAM as needed.  Therefore the PLUGIN is read only for accessing
    remote pretty libraries.  If you want to support writing to the repo, then you
    could use the above API.
*/


#include <sstream>
#include <boost/ptr_container/ptr_map.hpp>
#include <avhttp.hpp>                       // chinese SSL magic
#include <wx/zipstrm.h>
#include <wx/mstream.h>
#include <wx/uri.h>

#include <macros.h>
#include <fctsys.h>
#include <io_mgr.h>
#include <richio.h>
#include <pcb_parser.h>
#include <class_board.h>
#include <github_plugin.h>

using namespace std;

typedef boost::ptr_map<std::string, wxZipEntry>     MODULE_MAP;
typedef MODULE_MAP::iterator                        MODULE_ITER;
typedef MODULE_MAP::const_iterator                  MODULE_CITER;


/**
 * Class GH_CACHE
 * assists only within GITHUB_PLUGIN and hold a map of footprint name to wxZipEntry
 */
struct GH_CACHE : public MODULE_MAP
{
    // MODULE_MAP is a boost::ptr_map template, made into a class hereby.
};


GITHUB_PLUGIN::GITHUB_PLUGIN() :
    m_cache( 0 )
{
}


GITHUB_PLUGIN::~GITHUB_PLUGIN()
{
    delete m_cache;
}


const wxString& GITHUB_PLUGIN::PluginName() const
{
    static wxString name( wxT( "Github" ) );
    return name;
}


const wxString& GITHUB_PLUGIN::GetFileExtension() const
{
    static wxString empty_ext;
    return empty_ext;
}


wxArrayString GITHUB_PLUGIN::FootprintEnumerate(
        const wxString& aLibraryPath, PROPERTIES* aProperties )
{
    cacheLib( aLibraryPath );

    wxArrayString   ret;

    for( MODULE_ITER it = m_cache->begin();  it!=m_cache->end();  ++it )
    {
        ret.Add( FROM_UTF8( it->first.c_str() ) );
    }

    return ret;
}


MODULE* GITHUB_PLUGIN::FootprintLoad( const wxString& aLibraryPath,
        const wxString& aFootprintName, PROPERTIES* aProperties )
{
    cacheLib( aLibraryPath );

    string fp_name = TO_UTF8( aFootprintName );

    MODULE_CITER it = m_cache->find( fp_name );

    if( it != m_cache->end() )  // fp_name is present
    {
        wxMemoryInputStream mis( &m_zip_image[0], m_zip_image.size() );

        // This decoder should always be UTF8, since it was saved that way by git.
        // That is, since pretty footprints are UTF8, and they were pushed to the
        // github repo, they are still UTF8.
        wxZipInputStream    zis( mis, wxConvUTF8 );
        wxZipEntry*         entry = (wxZipEntry*) it->second;   // remove "const"-ness

        if( zis.OpenEntry( *entry ) )
        {
            INPUTSTREAM_LINE_READER reader( &zis );
            PCB_PARSER              parser( &reader );

            MODULE* ret = (MODULE*) parser.Parse();

            return ret;
        }
    }

    return NULL;    // this API function returns NULL for "not found", per spec.
}


bool GITHUB_PLUGIN::IsFootprintLibWritable( const wxString& aLibraryPath )
{
    return false;
}


void GITHUB_PLUGIN::cacheLib( const wxString& aLibraryPath ) throw( IO_ERROR )
{
    if( !m_cache || m_lib_path != aLibraryPath )
    {
        delete m_cache;
        m_cache = new GH_CACHE();
        remote_get_zip( aLibraryPath );
        m_lib_path = aLibraryPath;

        wxMemoryInputStream mis( &m_zip_image[0], m_zip_image.size() );

        // @todo: generalize this name encoding from a PROPERTY (option) later
        wxZipInputStream    zis( mis, wxConvUTF8 );

        wxZipEntry* entry;

        while( (entry = zis.GetNextEntry()) != NULL )
        {
            wxFileName  fn( entry->GetName() );

            if( fn.GetExt() == wxT( "kicad_mod" ) )
            {
                string fp_name = TO_UTF8( fn.GetName() );

                m_cache->insert( fp_name, entry );
            }
            else
                delete entry;
        }
    }
}


bool GITHUB_PLUGIN::repoURL_zipURL( const wxString& aRepoURL, string* aZipURL )
{
    // e.g. "https://github.com/liftoff-sr/pretty_footprints"
    D(printf("aRepoURL:%s\n", TO_UTF8( aRepoURL ) );)

    wxURI   repo( aRepoURL );

    if( repo.HasServer() && repo.HasPath() )
    {
        // goal: "https://github.com/liftoff-sr/pretty_footprints/archive/master.zip"
        wxString    zip_url( wxT("https://") );

        zip_url += repo.GetServer();
        zip_url += repo.GetPath();
        zip_url += wxT('/');
        zip_url += wxT( "archive/master.zip" );

        *aZipURL = zip_url.utf8_str();
        return true;
    }
    return false;
}


void GITHUB_PLUGIN::remote_get_zip( const wxString& aRepoURL ) throw( IO_ERROR )
{
    string  zip_url;

    if( !repoURL_zipURL( aRepoURL, &zip_url ) )
    {
        wxString msg = wxString::Format( _("Unable to parse URL: %s"), GetChars( m_lib_path ) );
        THROW_IO_ERROR( msg );
    }

    boost::asio::io_service io;
    avhttp::http_stream     h( io );
    avhttp::request_opts    options;

    options.insert( "Accept",       "application/zip" );
    options.insert( "User-Agent",   "http://kicad-pcb.org" );   // THAT WOULD BE ME.
    h.request_options( options );

    try
    {
        h.open( zip_url );      // only one file, therefore do it synchronously.
        ostringstream os;
        os << &h;

        // Keep zip file byte image in RAM.  That plus the MODULE_MAP will constitute
        // the cache.  We don't cache the MODULEs per se, we parse those as needed from
        // this zip file image.
        m_zip_image = os.str();

        // 4 lines, using SSL, top that.
    }
    catch( std::exception& e )
    {
        THROW_IO_ERROR( e.what() );
    }
}


#if 0 && defined(STANDALONE)

int main( int argc, char** argv )
{
   INIT_LOGGER( ".", "test.log" );

    GITHUB_PLUGIN gh;

    try
    {
        wxArrayString fps = gh.FootprintEnumerate(
                wxT( "https://github.com/liftoff-sr/pretty_footprints" ),
                NULL
                );

        for( int i=0; i<(int)fps.Count(); ++i )
        {
            printf("[%d]:%s\n", i, TO_UTF8( fps[i] ) );
        }
    }
    catch( IO_ERROR ioe )
    {
        printf( "%s\n", TO_UTF8(ioe.errorText) );
    }

    UNINIT_LOGGER();

    return 0;
}

#endif

