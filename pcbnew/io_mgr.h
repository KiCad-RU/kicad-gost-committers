#ifndef IO_MGR_H_
#define IO_MGR_H_

/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2011-2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2011 Kicad Developers, see change_log.txt for contributors.
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

#include <richio.h>
#include <hashtables.h>

class BOARD;
class PLUGIN;
class MODULE;


/**
 * Class IO_MGR
 * is a factory which returns an instance of a PLUGIN.
 */
class IO_MGR
{
public:

    /**
     * Enum PCB_FILE_T
     * is a set of file types that the IO_MGR knows about, and for which there
     * has been a plugin written.
     */
    enum PCB_FILE_T
    {
        LEGACY,             //< Legacy Pcbnew file formats prior to s-expression.
        KICAD,              //< S-expression Pcbnew file format.
        EAGLE,
        PCAD,
        GEDA_PCB,           //< Geda PCB file formats.

        // add your type here.

        // ALTIUM,
        // etc.
    };

    /**
     * Function PluginFind
     * returns a PLUGIN which the caller can use to import, export, save, or load
     * design documents.  The returned PLUGIN, may be reference counted, so please
     * call PluginRelease() when you are done using the returned PLUGIN.  It may or
     * may not be code running from a DLL/DSO.
     *
     * @param aFileType is from PCB_FILE_T and tells which plugin to find.
     *
     * @return PLUGIN* - the plugin corresponding to aFileType or NULL if not found.
     *  Caller owns the returned object, and must call PluginRelease when done using it.
     */
    static PLUGIN* PluginFind( PCB_FILE_T aFileType );

    /**
     * Function PluginRelease
     * releases a PLUGIN back to the system, and may cause it to be unloaded from memory.
     *
     * @param aPlugin is the one to be released, and which is no longer usable
     *  after calling this.
     */
    static void PluginRelease( PLUGIN* aPlugin );

    /**
     * Function ShowType
     * returns a brief name for a plugin, given aFileType enum.
     */
    static const wxString ShowType( PCB_FILE_T aFileType );

    /**
     * Function EnumFromStr
     * returns the PCB_FILE_T from the corresponding plugin type name: "kicad", "legacy", etc.
     */
    static PCB_FILE_T EnumFromStr( const wxString& aFileType );

    /**
     * Function GetFileExtension
     * returns the file extension for \a aFileType.
     *
     * @param aFileType The #PCB_FILE_T type.
     * @return A wxString object containing the file extension for \a aFileType or an empty
     *         string if \a aFileType is invalid.
     */
    static const wxString GetFileExtension( PCB_FILE_T aFileType );

    /**
     * Function GuessPluginTypeFromLibPath
     * returns a plugin type given a footprint library's libPath.
     */
    static PCB_FILE_T GuessPluginTypeFromLibPath( const wxString& aLibPath );

    /**
     * Function Load
     * finds the requested PLUGIN and if found, calls the PLUGIN->Load(..) funtion
     * on it using the arguments passed to this function.  After the PLUGIN->Load()
     * function returns, the PLUGIN is Released() as part of this call.
     *
     * @param aFileType is the PCB_FILE_T of file to load.
     *
     * @param aFileName is the name of the file to load.
     *
     * @param aAppendToMe is an existing BOARD to append to, use NULL if fresh
     *  board load is wanted.
     *
     * @param aProperties is an associative array that allows the caller to
     *  pass additional tuning parameters to the PLUGIN.
     *
     * @return BOARD* - caller owns it, never NULL because exception thrown if error.
     *
     * @throw IO_ERROR if the PLUGIN cannot be found, file cannot be found,
     *  or file cannot be loaded.
     */
    static BOARD* Load( PCB_FILE_T aFileType, const wxString& aFileName,
                        BOARD* aAppendToMe = NULL, PROPERTIES* aProperties = NULL );

    /**
     * Function Save
     * will write either a full aBoard to a storage file in a format that this
     * implementation knows about, or it can be used to write a portion of
     * aBoard to a special kind of export file.
     *
     * @param aFileType is the PCB_FILE_T of file to save.
     *
     * @param aFileName is the name of a file to save to on disk.
     * @param aBoard is the BOARD document (data tree) to save or export to disk.
     *
     * @param aBoard is the in memory document tree from which to extract information
     *  when writing to \a aFileName.  The caller continues to own the BOARD, and
     *  the plugin should refrain from modifying the BOARD if possible.
     *
     * @param aProperties is an associative array that can be used to tell the
     *  saver how to save the file, because it can take any number of
     *  additional named tuning arguments that the plugin is known to support.
     *  The caller continues to own this object (plugin may not delete it), and
     *  plugins should expect it to be optionally NULL.
     *
     * @throw IO_ERROR if there is a problem saving or exporting.
     */
    static void Save( PCB_FILE_T aFileType, const wxString& aFileName,
                      BOARD* aBoard, PROPERTIES* aProperties = NULL );
};


/**
 * Class PLUGIN
 * is a base class that BOARD loading and saving plugins should derive from.
 * Implementations can provide either Load() or Save() functions, or both.
 * PLUGINs throw exceptions, so it is best that you wrap your calls to these
 * functions in a try catch block.  Plugins throw exceptions because it is illegal
 * for them to have any user interface calls in them whatsoever, i.e. no windowing
 * or screen printing at all.
 *
 * <pre>
 *   try
 *   {
 *        IO_MGR::Load(...);
 *   or
 *        IO_MGR::Save(...);
 *   }
 *   catch( IO_ERROR ioe )
 *   {
 *        // grab text from ioe, show in error window.
 *   }
 * </pre>
 */
class PLUGIN
{
public:

    //-----<PUBLIC PLUGIN API>-------------------------------------------------

    /**
     * Function PluginName
     * returns a brief hard coded name for this PLUGIN.
     */
    virtual const wxString& PluginName() const = 0;

    /**
     * Function GetFileExtension
     * returns the file extension for the PLUGIN.
     */
    virtual const wxString& GetFileExtension() const = 0;

    /**
     * Function Load
     * loads information from some input file format that this PLUGIN implementation
     * knows about, into either a new BOARD or an existing one. This may be used to load an
     * entire new BOARD, or to augment an existing one if @a aAppendToMe is not NULL.
     *
     * @param aFileName is the name of the file to use as input and may be foreign in
     *  nature or native in nature.
     *
     * @param aAppendToMe is an existing BOARD to append to, but if NULL then
     *   this means "do not append, rather load anew".
     *
     * @param aProperties is an associative array that can be used to tell the
     *  loader how to load the file, because it can take any number of
     *  additional named arguments that the plugin is known to support. These are
     *  tuning parameters for the import or load.  The caller continues to own
     *  this object (plugin may not delete it), and plugins should expect it to
     *  be optionally NULL.
     *
     * @return BOARD* - the successfully loaded board, or the same one as aAppendToMe
     *  if aAppendToMe was not NULL, and caller owns it.
     *
     * @throw IO_ERROR if there is a problem loading, and its contents should
     *  say what went wrong, using line number and character offsets of the
     *  input file if possible.
     */
    virtual BOARD* Load( const wxString& aFileName, BOARD* aAppendToMe,
                         PROPERTIES* aProperties = NULL );

    /**
     * Function Save
     * will write @a aBoard to a storage file in a format that this
     * PLUGIN implementation knows about, or it can be used to write a portion of
     * aBoard to a special kind of export file.
     *
     * @param aFileName is the name of a file to save to on disk.
     *
     * @param aBoard is the class BOARD in memory document tree from which to
     *  extract information when writing to \a aFileName.  The caller continues to
     *  own the BOARD, and the plugin should refrain from modifying the BOARD if possible.
     *
     * @param aProperties is an associative array that can be used to tell the
     *  saver how to save the file, because it can take any number of
     *  additional named tuning arguments that the plugin is known to support.
     *  The caller continues to own this object (plugin may not delete it),
     *  and plugins should expect it to be optionally NULL.
     *
     * @throw IO_ERROR if there is a problem saving or exporting.
     */
    virtual void Save( const wxString& aFileName, BOARD* aBoard,
                       PROPERTIES* aProperties = NULL );

    //-----<Footprint Stuff>-----------------------------

    /**
     * Function FootprintEnumerate
     * returns a list of footprint names contained within the library at @a aLibraryPath.
     *
     * @param aLibraryPath is a locator for the "library", usually a directory
     *   or file containing several footprints.
     *
     * @param aProperties is an associative array that can be used to tell the
     *  plugin how to access the library.
     *  The caller continues to own this object (plugin may not delete it), and
     *  plugins should expect it to be optionally NULL.
     *
     * @return wxArrayString - is the array of available footprint names inside
     *   a library
     *
     * @throw IO_ERROR if the library cannot be found, or footprint cannot be loaded.
     */
    virtual wxArrayString FootprintEnumerate( const wxString& aLibraryPath,
                                              PROPERTIES*     aProperties = NULL);

    /**
     * Function FootprintLoad
     * loads a footprint having @a aFootprintName from the @a aLibraryPath containing
     * a library format that this PLUGIN knows about.
     *
     * @param aLibraryPath is a locator for the "library", usually a directory
     *   or file containing several footprints.
     *
     * @param aFootprintName is the name of the footprint to load.
     *
     * @param aProperties is an associative array that can be used to tell the
     *  loader implementation to do something special, because it can take any number of
     *  additional named tuning arguments that the plugin is known to support.
     *  The caller continues to own this object (plugin may not delete it), and
     *  plugins should expect it to be optionally NULL.
     *
     * @return  MODULE* - if found caller owns it, else NULL if not found.
     *
     * @throw   IO_ERROR if the library cannot be found or read.  No exception
     *          is thrown in the case where aFootprintName cannot be found.
     */
    virtual MODULE* FootprintLoad( const wxString& aLibraryPath, const wxString& aFootprintName,
                                    PROPERTIES* aProperties = NULL );

    /**
     * Function FootprintSave
     * will write @a aModule to an existing library located at @a aLibraryPath.
     * If a footprint by the same name already exists, it is replaced.
     *
     * @param aLibraryPath is a locator for the "library", usually a directory
     *      or file containing several footprints. This is where the footprint is
     *      to be stored.
     *
     * @param aFootprint is what to store in the library. The caller continues
     *    to own the footprint after this call.
     *
     * @param aProperties is an associative array that can be used to tell the
     *  saver how to save the footprint, because it can take any number of
     *  additional named tuning arguments that the plugin is known to support.
     *  The caller continues to own this object (plugin may not delete it), and
     *  plugins should expect it to be optionally NULL.
     *
     * @throw IO_ERROR if there is a problem saving.
     */
    virtual void FootprintSave( const wxString& aLibraryPath, const MODULE* aFootprint,
                                    PROPERTIES* aProperties = NULL );

    /**
     * Function FootprintDelete
     * deletes the @a aFootprintName from the library at @a aLibraryPath.
     *
     * @param aLibraryPath is a locator for the "library", usually a directory
     *   or file containing several footprints.
     *
     * @param aFootprintName is the name of a footprint to delete from the specified library.
     *
     * @throw IO_ERROR if there is a problem finding the footprint or the library, or deleting it.
     */
    virtual void FootprintDelete( const wxString& aLibraryPath, const wxString& aFootprintName );

    /**
     * Function FootprintLibCreate
     * creates a new empty footprint library at @a aLibraryPath empty.  It is an
     * error to attempt to create an existing library or to attempt to create
     * on a "read only" location.
     *
     * @param aLibraryPath is a locator for the "library", usually a directory
     *   or file which will contain footprints.
     *
     * @param aProperties is an associative array that can be used to tell the
     *  library create function anything special, because it can take any number of
     *  additional named tuning arguments that the plugin is known to support.
     *  The caller continues to own this object (plugin may not delete it), and
     *  plugins should expect it to be optionally NULL.
     *
     * @throw IO_ERROR if there is a problem finding the library, or creating it.
     */
    virtual void FootprintLibCreate( const wxString& aLibraryPath, PROPERTIES* aProperties = NULL );

    /**
     * Function FootprintLibDelete
     * deletes an existing footprint library and returns true, or if library does not
     * exist returns false, or throws an exception if library exists but is read only or
     * cannot be deleted for some other reason.
     *
     * @param aLibraryPath is a locator for the "library", usually a directory
     *   or file which will contain footprints.
     *
     * @param aProperties is an associative array that can be used to tell the
     *  library delete implementation function anything special, because it can
     *  take any number of additional named tuning arguments that the plugin is
     *  known to support. The caller continues to own this object (plugin may
     *  not delete it), and plugins should expect it to be optionally NULL.
     *
     * @return bool - true if library deleted, false if library did not exist.
     *
     * @throw IO_ERROR if there is a problem deleting an existing library.
     */
    virtual bool FootprintLibDelete( const wxString& aLibraryPath, PROPERTIES* aProperties = NULL );

    /**
     * Function IsFootprintLibWritable
     * returns true iff the library at @a aLibraryPath is writable.  (Often
     * system libraries are read only because of where they are installed.)
     *
     * @throw IO_ERROR if no library at aLibraryPath exists.
     */
    virtual bool IsFootprintLibWritable( const wxString& aLibraryPath );

    //-----</PUBLIC PLUGIN API>------------------------------------------------


    /*  The compiler writes the "zero argument" constructor for a PLUGIN
        automatically if you do not provide one. If you decide you need to
        provide a zero argument constructor of your own design, that is allowed.
        It must be public, and it is what the IO_MGR uses.  Parameters may be
        passed into a PLUGIN via the PROPERTIES variable for any of the public
        API functions which take one.
    */

    virtual ~PLUGIN() {}

    /**
     * Class RELEASER
     * releases a PLUGIN in the context of a potential thrown exception, through
     * its destructor.
     */
    class RELEASER
    {
        PLUGIN* plugin;

    public:
        RELEASER( PLUGIN* aPlugin = NULL ) :
            plugin( aPlugin )
        {
        }

        ~RELEASER()
        {
            if( plugin )
                IO_MGR::PluginRelease( plugin );
        }

        operator PLUGIN* ()
        {
            return plugin;
        }

        PLUGIN* operator -> ()
        {
            return plugin;
        }
    };
};

#endif // IO_MGR_H_
