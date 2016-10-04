#ifndef _SCH_IO_MGR_H_
#define _SCH_IO_MGR_H_

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * Copyright (C) 2016 KiCad Developers, see CHANGELOG.TXT for contributors.
 *
 * @author Wayne Stambaugh <stambaughw@gmail.com>
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
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <richio.h>
#include <map>


class SCH_SHEET;
class SCH_SCREEN;
class SCH_PLUGIN;
class KIWAY;
class LIB_PART;
class LIB_ALIAS;
class PART_LIB;
class PROPERTIES;


/**
 * Class SCH_IO_MGR
 * is a factory which returns an instance of a #SCH_PLUGIN.
 */
class SCH_IO_MGR
{
public:

    /**
     * Enum SCH_FILE_T
     * is a set of file types that the SCH_IO_MGR knows about, and for which there
     * has been a plugin written.
     */
    enum SCH_FILE_T
    {
        SCH_LEGACY,      ///< Legacy Eeschema file formats prior to s-expression.
        SCH_KICAD,       ///< The s-expression version of the schematic file formats.
        // Add your schematic type here.

        // ALTIUM,
        // etc.
    };

    /**
     * Function FindPlugin
     * returns a SCH_PLUGIN which the caller can use to import, export, save, or load
     * design documents.  The returned SCH_PLUGIN, may be reference counted, so please
     * call PluginRelease() when you are done using the returned SCH_PLUGIN.  It may or
     * may not be code running from a DLL/DSO.
     *
     * @param aFileType is from SCH_FILE_T and tells which plugin to find.
     *
     * @return SCH_PLUGIN* - the plugin corresponding to aFileType or NULL if not found.
     *  Caller owns the returned object, and must call PluginRelease when done using it.
     */
    static SCH_PLUGIN* FindPlugin( SCH_FILE_T aFileType );

    /**
     * Function PluginRelease
     * releases a SCH_PLUGIN back to the system, and may cause it to be unloaded from memory.
     *
     * @param aPlugin is the one to be released, and which is no longer usable
     *  after calling this.
     */
    static void ReleasePlugin( SCH_PLUGIN* aPlugin );

    /**
     * Function ShowType
     * returns a brief name for a plugin, given aFileType enum.
     */
    static const wxString ShowType( SCH_FILE_T aFileType );

    /**
     * Function EnumFromStr
     * returns the SCH_FILE_T from the corresponding plugin type name: "kicad", "legacy", etc.
     */
    static SCH_FILE_T EnumFromStr( const wxString& aFileType );

    /**
     * Function GetFileExtension
     * returns the file extension for \a aFileType.
     *
     * @param aFileType The #SCH_FILE_T type.
     * @return A wxString object containing the file extension for \a aFileType or an empty
     *         string if \a aFileType is invalid.
     */
    static const wxString GetFileExtension( SCH_FILE_T aFileType );

    /**
     * Function GuessPluginTypeFromLibPath
     * returns a plugin type given a footprint library's libPath.
     */
    static SCH_FILE_T GuessPluginTypeFromLibPath( const wxString& aLibPath );

    /**
     * Function Load
     * finds the requested SCH_PLUGIN and if found, calls the SCH_PLUGIN->Load(..) function
     * on it using the arguments passed to this function.  After the SCH_PLUGIN->Load()
     * function returns, the SCH_PLUGIN is Released() as part of this call.
     *
     * @param aFileType is the SCH_FILE_T of file to load.
     * @param aFileName is the name of the file to load.
     * @param aKiway is the #KIWAY object used to access the component libraries loaded
     *               by the project.
     * @param aAppendToMe is an existing #SCHEMATIC to append to, use NULL if a new
     *                    #SCHEMATIC load is wanted.
     * @param aProperties is an associative array that allows the caller to pass additional
     *                    tuning parameters to the SCH_PLUGIN.
     *
     * @return SCHEMATIC* - caller owns it, never NULL because exception thrown if error.
     *
     * @throw IO_ERROR if the SCH_PLUGIN cannot be found, file cannot be found,
     *                 or file cannot be loaded.
     */
    static SCH_SHEET* Load( SCH_FILE_T aFileType, const wxString& aFileName, KIWAY* aKiway,
                            SCH_SHEET* aAppendToMe = NULL, const PROPERTIES* aProperties = NULL );

    /**
     * Function Save
     * will write either a full aSchematic to a storage file in a format that this
     * implementation knows about, or it can be used to write a portion of
     * aSchematic to a special kind of export file.
     *
     * @param aFileType is the SCH_FILE_T of file to save.
     *
     * @param aFileName is the name of a file to save to on disk.
     * @param aSchematic is the SCHEMATIC document (data tree) to save or export to disk.
     *
     * @param aSchematic is the in memory document tree from which to extract information
     *  when writing to \a aFileName.  The caller continues to own the SCHEMATIC, and
     *  the plugin should refrain from modifying the SCHEMATIC if possible.
     *
     * @param aProperties is an associative array that can be used to tell the
     *  saver how to save the file, because it can take any number of
     *  additional named tuning arguments that the plugin is known to support.
     *  The caller continues to own this object (plugin may not delete it), and
     *  plugins should expect it to be optionally NULL.
     *
     * @throw IO_ERROR if there is a problem saving or exporting.
     */
    static void Save( SCH_FILE_T aFileType, const wxString& aFileName,
                      SCH_SCREEN* aSchematic, KIWAY* aKiway, const PROPERTIES* aProperties = NULL );
};


/**
 * Class SCH_PLUGIN
 * is a base class that SCHEMATIC loading and saving plugins should derive from.
 * Implementations can provide either Load() or Save() functions, or both.
 * SCH_PLUGINs throw exceptions, so it is best that you wrap your calls to these
 * functions in a try catch block.  Plugins throw exceptions because it is illegal
 * for them to have any user interface calls in them whatsoever, i.e. no windowing
 * or screen printing at all.
 *
 * <pre>
 *   try
 *   {
 *        SCH_IO_MGR::Load(...);
 *   or
 *        SCH_IO_MGR::Save(...);
 *   }
 *   catch( const IO_ERROR& ioe )
 *   {
 *        // grab text from ioe, show in error window.
 *   }
 * </pre>
 */
class SCH_PLUGIN
{
public:

    //-----<PUBLIC SCH_PLUGIN API>-------------------------------------------------

    /**
     * Function GetName
     * returns a brief hard coded name for this SCH_PLUGIN.
     */
    virtual const wxString GetName() const = 0;

    /**
     * Function GetFileExtension
     * returns the file extension for the SCH_PLUGIN.
     */
    virtual const wxString GetFileExtension() const = 0;

    /**
     * Function Load
     * loads information from some input file format that this SCH_PLUGIN implementation
     * knows about, into either a new SCHEMATIC or an existing one. This may be used to load an
     * entire new SCHEMATIC, or to augment an existing one if @a aAppendToMe is not NULL.
     *
     * @param aFileName is the name of the file to use as input and may be foreign in
     *  nature or native in nature.
     *
     * @param aAppendToMe is an existing SCHEMATIC to append to, but if NULL then
     *   this means "do not append, rather load anew".
     *
     * @param aProperties is an associative array that can be used to tell the
     *  loader how to load the file, because it can take any number of
     *  additional named arguments that the plugin is known to support. These are
     *  tuning parameters for the import or load.  The caller continues to own
     *  this object (plugin may not delete it), and plugins should expect it to
     *  be optionally NULL.
     *
     * @return SCHEMATIC* - the successfully loaded schematic, or the same one as \a aAppendToMe
     *  if \a aAppendToMe was not NULL, and the caller owns it.
     *
     * @throw IO_ERROR if there is a problem loading, and its contents should
     *  say what went wrong, using line number and character offsets of the
     *  input file if possible.
     */
    virtual SCH_SHEET* Load( const wxString& aFileName, KIWAY* aKiway,
                             SCH_SHEET* aAppendToMe = NULL, const PROPERTIES* aProperties = NULL );

    /**
     * Function Save
     * will write @a aSchematic to a storage file in a format that this
     * SCH_PLUGIN implementation knows about, or it can be used to write a portion of
     * aSchematic to a special kind of export file.
     *
     * @param aFileName is the name of a file to save to on disk.
     *
     * @param aSchematic is the class SCHEMATIC in memory document tree from which to
     *  extract information when writing to \a aFileName.  The caller continues to
     *  own the SCHEMATIC, and the plugin should refrain from modifying the SCHEMATIC if possible.
     *
     * @param aProperties is an associative array that can be used to tell the
     *  saver how to save the file, because it can take any number of
     *  additional named tuning arguments that the plugin is known to support.
     *  The caller continues to own this object (plugin may not delete it),
     *  and plugins should expect it to be optionally NULL.
     *
     * @throw IO_ERROR if there is a problem saving or exporting.
     */
    virtual void Save( const wxString& aFileName, SCH_SCREEN* aSchematic, KIWAY* aKiway,
                       const PROPERTIES* aProperties = NULL );

    /**
     * Function EnumerateSymbolLib
     * returns a list of #LIB_PART alias names contained within the library @a aLibraryPath.
     *
     * @param aAliasNameList is an array to populate with the #LIB_ALIAS names associated with
     *                       the library.
     * @param aLibraryPath is a locator for the "library", usually a directory, file,
     *                     or URL containing one or more #LIB_PART objects.
     * @param aProperties is an associative array that can be used to tell the plugin anything
     *                    needed about how to perform with respect to @a aLibraryPath.  The
     *                    caller continues to own this object (plugin may not delete it), and
     *                    plugins should expect it to be optionally NULL.
     *
     * @throw IO_ERROR if the library cannot be found, the part library cannot be loaded.
     */
    virtual void EnumerateSymbolLib( wxArrayString&    aAliasNameList,
                                     const wxString&   aLibraryPath,
                                     const PROPERTIES* aProperties = NULL );

    // Temporary for testing using PART_LIB instead of SCH_PLUGIN.
    virtual void TransferCache( PART_LIB& aTarget );

    /**
     * Function LoadSymbol
     * loads a #LIB_ALIAS object having @a aAliasName from the @a aLibraryPath containing
     * a library format that this SCH_PLUGIN knows about.  The #LIB_PART should be accessed
     * indirectly using the #LIB_ALIAS it is associated with.
     *
     * @param aLibraryPath is a locator for the "library", usually a directory, file,
     *                     or URL containing several symbols.
     *
     * @param aAliasName is the alias name of the #LIB_PART to load.
     *
     * @param aProperties is an associative array that can be used to tell the loader
     *                    implementation to do something special, because it can take
     *                    any number of additional named tuning arguments that the plugin
     *                    is known to support.  The caller continues to own this object
     *                    (plugin may not delete it), and plugins should expect it to be
     *                    optionally NULL.
     *
     * @return  LIB_ALIAS* - if found caller shares it, else NULL if not found.
     *
     * @throw   IO_ERROR if the library cannot be found or read.  No exception
     *                   is thrown in the case where aAliasName cannot be found.
     */
    virtual LIB_ALIAS* LoadSymbol( const wxString& aLibraryPath, const wxString& aAliasName,
                                   const PROPERTIES* aProperties = NULL );

    /**
     * Function SaveSymbol
     * will write @a aSymbol to an existing library located at @a aLibraryPath.
     * If a #LIB_PART by the same name already exists or there are any conflicting
     * alias names, an exception is thrown.  Symbol libraries cannot have duplicate
     * alias names.
     *
     * @param aLibraryPath is a locator for the "library", usually a directory, file,
     *                     or URL containing several symbols.
     *
     * @param aSymbol is what to store in the library.  The library is refreshed and the
     *                caller must update any #LIB_PART pointers that may have changed.
     *
     * @param aProperties is an associative array that can be used to tell the
     *                    saver how to save the symbol, because it can take any number of
     *                    additional named tuning arguments that the plugin is known to support.
     *                    The caller continues to own this object (plugin may not delete it), and
     *                    plugins should expect it to be optionally NULL.
     *
     * @throw IO_ERROR if there is a problem saving or duplicate alias names.
     */
    virtual void SaveSymbol( const wxString& aLibraryPath, const LIB_PART* aSymbol,
                             const PROPERTIES* aProperties = NULL );

    /**
     * Function DeleteAlias
     * deletes @a aAliasName from the library at @a aLibraryPath.
     *
     * If @a aAliasName refers the the root #LIB_PART object, the part is renamed to
     * the next or previous #LIB_ALIAS in the #LIB_PART if one exists.  If the #LIB_ALIAS
     * is the last alias referring to the root #LIB_PART, the #LIB_PART is also removed
     * from the library.
     *
     * @param aLibraryPath is a locator for the "library", usually a directory, file,
     *                     or URL containing several symbols.
     *
     * @param aAliasName is the name of a #LIB_ALIAS to delete from the specified library.
     *
     * @param aProperties is an associative array that can be used to tell the library
     *                    delete function anything special, because it can take any number
     *                    of additional named tuning arguments that the plugin is known to
     *                    support.  The caller continues to own this object (plugin may not
     *                    delete it), and plugins should expect it to be optionally NULL.
     *
     * @throw IO_ERROR if there is a problem finding the alias or the library or deleting it.
     */
    virtual void DeleteAlias( const wxString& aLibraryPath, const wxString& aAliasName,
                              const PROPERTIES* aProperties = NULL );

    /**
     * Function DeleteSymbol
     * deletes the entire #LIB_PART associated with @a aAliasName from the library
     * @a aLibraryPath.
     *
     * @param aLibraryPath is a locator for the "library", usually a directory, file,
     *                     or URL containing several symbols.
     *
     * @param aAliasName is the name of a #LIB_ALIAS associated with it's root #LIB_PART
     *                   object to delete from the specified library.
     *
     * @param aProperties is an associative array that can be used to tell the library
     *                    delete function anything special, because it can take any number
     *                    of additional named tuning arguments that the plugin is known to
     *                    support.  The caller continues to own this object (plugin may not
     *                    delete it), and plugins should expect it to be optionally NULL.
     *
     * @throw IO_ERROR if there is a problem finding the alias or the library or deleting it.
     */
    virtual void DeleteSymbol( const wxString& aLibraryPath, const wxString& aAliasName,
                               const PROPERTIES* aProperties = NULL );

    /**
     * Function SymbolLibCreate
     * creates a new empty footprint library at @a aLibraryPath empty.  It is an
     * error to attempt to create an existing library or to attempt to create
     * on a "read only" location.
     *
     * @param aLibraryPath is a locator for the "library", usually a directory, file,
     *   or URL containing several footprints.
     *
     * @param aProperties is an associative array that can be used to tell the
     *  library create function anything special, because it can take any number of
     *  additional named tuning arguments that the plugin is known to support.
     *  The caller continues to own this object (plugin may not delete it), and
     *  plugins should expect it to be optionally NULL.
     *
     * @throw IO_ERROR if there is a problem finding the library, or creating it.
     */
    virtual void SymbolLibCreate( const wxString& aLibraryPath,
                                  const PROPERTIES* aProperties = NULL );

    /**
     * Function SymbolLibDelete
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
    virtual bool SymbolLibDelete( const wxString& aLibraryPath,
                                  const PROPERTIES* aProperties = NULL );

    /**
     * Function IsSymbolLibWritable
     * returns true if the library at @a aLibraryPath is writable.  (Often
     * system libraries are read only because of where they are installed.)
     *
     * @param aLibraryPath is a locator for the "library", usually a directory, file,
     *   or URL containing several footprints.
     *
     * @throw IO_ERROR if no library at aLibraryPath exists.
     */
    virtual bool IsSymbolLibWritable( const wxString& aLibraryPath );

    /**
     * Function SymbolLibOptions
     * appends supported SCH_PLUGIN options to @a aListToAppenTo along with
     * internationalized descriptions.  Options are typically appended so
     * that a derived SCH_PLUGIN can call its base class
     * function by the same name first, thus inheriting options declared there.
     * (Some base class options could pertain to all Symbol*() functions
     * in all derived SCH_PLUGINs.)  Note that since aListToAppendTo is a PROPERTIES
     * object, all options will be unique and last guy wins.
     *
     * @param aListToAppendTo holds a tuple of
     * <dl>
     *   <dt>option</dt>
     *   <dd>This eventually is what shows up into the fp-lib-table "options"
     *       field, possibly combined with others.</dd>
     *   <dt>internationalized description</dt>
     *   <dd>The internationalized description is displayed in DIALOG_FP_SCH_PLUGIN_OPTIONS.
     *      It may be multi-line and be quite explanatory of the option.</dd>
     *  </dl>
     * <br>
     *  In the future perhaps @a aListToAppendTo evolves to something capable of also
     *  holding a wxValidator for the cells in said dialog:
     *  http://forums.wxwidgets.org/viewtopic.php?t=23277&p=104180.
     *   This would require a 3 column list, and introducing wx GUI knowledge to
     *   SCH_PLUGIN, which has been avoided to date.
     */
    virtual void SymbolLibOptions( PROPERTIES* aListToAppendTo ) const;

    //-----</PUBLIC SCH_PLUGIN API>------------------------------------------------


    /*  The compiler writes the "zero argument" constructor for a SCH_PLUGIN
        automatically if you do not provide one. If you decide you need to
        provide a zero argument constructor of your own design, that is allowed.
        It must be public, and it is what the SCH_IO_MGR uses.  Parameters may be
        passed into a SCH_PLUGIN via the PROPERTIES variable for any of the public
        API functions which take one.
    */
    virtual ~SCH_PLUGIN() { }


    /**
     * Class SCH_PLUGIN_RELEASER
     * releases a SCH_PLUGIN in the context of a potential thrown exception, through
     * its destructor.
     */
    class SCH_PLUGIN_RELEASER
    {
        SCH_PLUGIN* plugin;

        // private assignment operator so it's illegal
        SCH_PLUGIN_RELEASER& operator=( SCH_PLUGIN_RELEASER& aOther ) { return *this; }

        // private copy constructor so it's illegal
        SCH_PLUGIN_RELEASER( const SCH_PLUGIN_RELEASER& aOther ) {}

    public:
        SCH_PLUGIN_RELEASER( SCH_PLUGIN* aPlugin = NULL ) :
            plugin( aPlugin )
        {
        }

        ~SCH_PLUGIN_RELEASER()
        {
            if( plugin )
                release();
        }

        void release()
        {
            SCH_IO_MGR::ReleasePlugin( plugin );
            plugin = NULL;
        }

        void set( SCH_PLUGIN* aPlugin )
        {
            if( plugin )
                release();
            plugin = aPlugin;
        }

        operator SCH_PLUGIN* () const
        {
            return plugin;
        }

        SCH_PLUGIN* operator -> () const
        {
            return plugin;
        }
    };
};

#endif // _SCH_IO_MGR_H_
