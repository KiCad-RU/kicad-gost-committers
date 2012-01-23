/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2008-2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2011 KiCad Developers, see change_log.txt for contributors.
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
 * @file class_library.h
 * @brief Definition for component library class.
 */

#ifndef CLASS_LIBRARY_H
#define CLASS_LIBRARY_H

#include <wx/filename.h>

#include <class_libentry.h>


class LINE_READER;
class OUTPUTFORMATTER;


/*
 * Component Library version and file header  macros.
 */
#define LIB_VERSION_MAJOR 2
#define LIB_VERSION_MINOR 3

/* Must be the first line of component library (.lib) files. */
#define LIBFILE_IDENT     "EESchema-LIBRARY Version"

#define LIB_VERSION( major, minor ) ( major * 100 + minor )

#define IS_LIB_CURRENT_VERSION( major, minor )              \
    (                                                       \
        LIB_VERSION( major1, minor1 ) ==                    \
        LIB_VERSION( LIB_VERSION_MAJOR, LIB_VERSION_MINOR)  \
    )

/*
 * Library versions 2.3 and lower use the old separate library (.lib) and
 * document (.dcm) files.  Component libraries after 2.3 merged the library
 * and document files into a single library file.  This macro checks if the
 * library version supports the old format
 */
#define USE_OLD_DOC_FILE_FORMAT( major, minor )                 \
    ( LIB_VERSION( major, minor ) <= LIB_VERSION( 2, 3 ) )

/* Must be the first line of component library document (.dcm) files. */
#define DOCFILE_IDENT     "EESchema-DOCLIB  Version 2.0"

#define DOC_EXT           wxT( "dcm" )


/* Helpers for creating a list of component libraries. */
class CMP_LIBRARY;
class wxRegEx;


typedef boost::ptr_vector< CMP_LIBRARY > CMP_LIBRARY_LIST;

extern bool operator<( const CMP_LIBRARY& item1, const CMP_LIBRARY& item2 );


/**
 * Class CMP_LIBRARY
 * is used to load, save, search, and otherwise manipulate
 * component library files.
 */
class CMP_LIBRARY
{
    int                type;            ///< Library type indicator.
    wxFileName         fileName;        ///< Library file name.
    wxDateTime         timeStamp;       ///< Library save time and date.
    int                versionMajor;    ///< Library major version number.
    int                versionMinor;    ///< Library minor version number.
    bool               isCache;         /**< False for the "standard" libraries,
                                             True for the library cache */
    wxString           header;          ///< first line of loaded library.
    bool               isModified;      ///< Library modification status.
    LIB_ALIAS_MAP      aliases;         ///< Map of aliases objects associated with the library.

    static CMP_LIBRARY_LIST libraryList;
    static wxArrayString    libraryListSortOrder;

    friend class LIB_COMPONENT;

public:
    CMP_LIBRARY( int aType, const wxFileName& aFileName );
    CMP_LIBRARY( int aType, const wxString& aFileName )
    {
        CMP_LIBRARY( aType, wxFileName( aFileName ) );
    }
    ~CMP_LIBRARY();

    /**
     * Function Save
     * writes library to \a aFormatter.
     *
     * @param aFormatter An #OUTPUTFORMATTER object to write the library to.
     * @return True if success writing to \a aFormatter.
     */
    bool Save( OUTPUTFORMATTER& aFormatter );

    /**
     * Function SaveDocs
     * write the library document information to \a aFormatter.
     *
     * @param aFormatter An #OUTPUTFORMATTER object to write the library documentation to.
     * @return True if success writing to \a aFormatter.
     */
    bool SaveDocs( OUTPUTFORMATTER& aFormatter );

    /**
     * Load library from file.
     *
     * @param aErrorMsg - Error message if load fails.
     * @return True if load was successful otherwise false.
     */
    bool Load( wxString& aErrorMsg );

    bool LoadDocs( wxString& aErrorMsg );

private:
    bool SaveHeader( OUTPUTFORMATTER& aFormatter );

    bool LoadHeader( LINE_READER& aLineReader );
    void LoadAliases( LIB_COMPONENT* aComponent );

public:
    /**
     * Get library entry status.
     *
     * @return True if there are no entries in the library.
     */
    bool IsEmpty() const
    {
        return aliases.empty();
    }

    /**
     * Function GetCount
     * returns the number of entries in the library.
     *
     * @return The number of component and alias entries.
     */
    int GetCount() const
    {
        return aliases.size();
    }

    bool IsModified() const
    {
        return isModified;
    }

    bool IsCache() const { return isCache; }

    void SetCache( void ) { isCache = true; }

    /**
     * Function IsReadOnly
     * @return true if current user does not have write access to the library file.
     */
    bool IsReadOnly() const { return !fileName.IsFileWritable(); }

    /**
     * Load a string array with the names of all the entries in this library.
     *
     * @param aNames - String array to place entry names into.
     * @param aSort - Sort names if true.
     * @param aMakeUpperCase - Force entry names to upper case.
     */
    void GetEntryNames( wxArrayString& aNames, bool aSort = true,
                        bool aMakeUpperCase =
#ifndef KICAD_KEEPCASE
                                              true
#else
                                              false
#endif
                        );

    /**
     * Load string array with entry names matching name and/or key word.
     *
     * This currently mimics the old behavior of calling KeyWordOk() and
     * WildCompareString().  The names array will be populated with the
     * library entry names that meat the search criteria on exit.
     *
     * @param aNames - String array to place entry names into.
     * @param aNameSearch - Name wild card search criteria.
     * @param aKeySearch - Key word search criteria.
     * @param aSort - Sort names if true.
     */
    void SearchEntryNames( wxArrayString& aNames,
                           const wxString& aNameSearch = wxEmptyString,
                           const wxString& aKeySearch = wxEmptyString,
                           bool aSort = true );

    /**
     * Find components in library by key word regular expression search.
     *
     * @param aNames - String array to place found component names into.
     * @param aRe - Regular expression used to search component key words.
     * @param aSort - Sort component name list.
     */
    void SearchEntryNames( wxArrayString& aNames, const wxRegEx& aRe, bool aSort = true );

    /**
     * Checks \a aComponent for name conflict in the library.
     *
     * @param aComponent - The component to check.
     * @return True if a conflict exists.  Otherwise false.
     */
    bool Conflicts( LIB_COMPONENT* aComponent );

    /**
     * Find entry by name.
     *
     * @param aName - Name of entry, case insensitive.
     * @return Entry if found.  NULL if not found.
     */
    LIB_ALIAS* FindEntry( const wxChar* aName );

    /**
     * Find component by \a aName.
     *
     * This is a helper for FindEntry so casting a LIB_ALIAS pointer to
     * a LIB_COMPONENT pointer is not required.
     *
     * @param aName - Name of component, case insensitive.
     * @return Component if found.  NULL if not found.
     */
    LIB_COMPONENT* FindComponent( const wxChar* aName );

    /**
     * Find alias by \a nName.
     *
     * This is a helper for FindEntry so casting a LIB_ALIAS pointer to
     * a LIB_ALIAS pointer is not required.
     *
     * @param aName - Name of alias, case insensitive.
     * @return Alias if found.  NULL if not found.
     */
    LIB_ALIAS* FindAlias( const wxChar* aName )
    {
        return (LIB_ALIAS*) FindEntry( aName );
    }

    /**
     * Add a new \a aAlias entry to the library.
     *
     * First check if a component or alias with the same name already exists
     * in the library and add alias if no conflict occurs.  Once the alias
     * is added to the library it is owned by the library.  Deleting the
     * alias pointer will render the library unstable.  Use RemoveEntry to
     * remove the alias from the library.
     *
     * @param aAlias - Alias to add to library.
     * @return True if alias added to library.  False if a conflict exists.
     */
    bool AddAlias( LIB_ALIAS* aAlias );

    /**
     * Add \a aComponent entry to library.
     * Note a component can have an alias list,
     * so these alias will be added in library.
     * Conflicts can happen if aliases are already existing.
     * User is asked to choose what alias is removed (existing, or new)
     * @param aComponent - Component to add.
     * @return Added component if successful.
     */
    LIB_COMPONENT* AddComponent( LIB_COMPONENT* aComponent );

    /**
     * Safely remove \a aEntry from the library and return the next entry.
     *
     * The next entry returned depends on the entry being removed.  If the entry being
     * remove also removes the component, then the next entry from the list is returned.
     * If the entry being used only removes an alias from a component, then the next alias
     * of the component is returned.
     *
     * @param aEntry - Entry to remove from library.
     * @return The next entry in the library or NULL if the library is empty.
     */
    LIB_ALIAS* RemoveEntry( LIB_ALIAS* aEntry );

    /**
     * Replace an existing component entry in the library.
     * Note a component can have an alias list,
     * so these alias will be added in library (and previously existing alias removed)
     * @param aOldComponent - The component to replace.
     * @param aNewComponent - The new component.
     */
    LIB_COMPONENT* ReplaceComponent( LIB_COMPONENT* aOldComponent,
                                     LIB_COMPONENT* aNewComponent );

    /**
     * Return the first entry in the library.
     *
     * @return The first entry or NULL if the library has no entries.
     */
    LIB_ALIAS* GetFirstEntry();

    /**
     * Find next library entry by \a aName.
     *
     * If the name of the entry is the last entry in the library, the first
     * entry in the list is returned.
     *
     * @param aName - Name of current entry.
     * @return Next entry if entry name is found. Otherwise NULL.
     */
    LIB_ALIAS* GetNextEntry( const wxChar* aName );


    /**
     * Find previous library entry by \a aName.
     *
     * If the name of the entry is the first entry in the library, the last
     * entry in the list is returned.
     *
     * @param aName - Name of current entry.
     * @return Previous entry if entry name is found, otherwise NULL.
     */
    LIB_ALIAS* GetPreviousEntry( const wxChar* aName );

    /**
     * Return the file name without path or extension.
     *
     * @return Name of library file.
     */
    wxString GetName() const { return fileName.GetName(); }

    /**
     * Function GetFullFileName
     * returns the full file library name with path and extension.
     *
     * @return wxString - Full library file name with path and extension.
     */
    wxString GetFullFileName() { return fileName.GetFullPath(); }

    /**
     * Function GetLogicalName
     * returns the logical name of the library.
     * @return wxString - The logical name of this library.
     */
    wxString GetLogicalName()
    {
        /*  for now is the filename without path or extension.

            Technically the library should not know its logical name!
            This will eventually come out of a pair of lookup tables using a
            reverse lookup using the full name or library pointer as a key.
            Search will be by project lookup table and then user lookup table if
            not found.
        */
        return fileName.GetName();
    }


    /**
     * Function SetFileName
     * sets the component library file name.
     *
     * @param aFileName - New library file name.
     */
    void SetFileName( const wxFileName aFileName )
    {
        if( aFileName != fileName )
            fileName = aFileName;
    }

    /*
     * The following static methods are for manipulating the list of
     * component libraries.  This eliminates the need for yet another
     * global variable ( formerly g_LibraryList ) and gives some measure
     * of safety from abusing the library list.
     */

    /**
     * Function LibraryExists
     * tests for existence of a library.
     *
     * @param aLibptr - aLibptr.
     * @return bool - true if exists, else false
     */

    static bool LibraryExists( const CMP_LIBRARY* aLibptr );

    /**
     * Function LoadLibrary
     * loads a component library file.
     *
     * @param aFileName - File name of the component library to load.
     * @param aErrorMsg - Error message if the component library failed to load.
     * @return Library object if library file loaded successfully,
     *         otherwise NULL.
     */
    static CMP_LIBRARY* LoadLibrary( const wxFileName& aFileName, wxString& aErrorMsg );

    /**
     * Function AddLibrary
     * adds a component library to the library list.
     *
     * @param aFileName - File name object of component library.
     * @param aErrorMsg - Error message if the component library failed to load.
     * @return True if library loaded properly otherwise false.
     */
    static bool AddLibrary( const wxFileName& aFileName, wxString& aErrorMsg );

    /**
     * Function AddLibrary
     * inserts a component library into the library list.
     *
     * @param aFileName - File name object of component library.
     * @param aErrorMsg - Error message if the component library failed to load.
     * @param aIterator - Iterator to insert library in front of.
     * @return True if library loaded properly otherwise false.
     */
    static bool AddLibrary( const wxFileName& aFileName, wxString& aErrorMsg,
                            CMP_LIBRARY_LIST::iterator& aIterator );

    /**
     * Function RemoveLibrary
     * removes a component library from the library list.
     *
     * @param aName - Name of component library to remove.
     */
    static void RemoveLibrary( const wxString& aName );

    static void RemoveAllLibraries() { libraryList.clear(); }

    /**
     * Function FindLibrary
     * finds a component library by \a aName.
     *
     * @param aName - Library file name without path or extension to find.
     * @return Component library if found, otherwise NULL.
     */
    static CMP_LIBRARY* FindLibrary( const wxString& aName );

    /**
     * Function GetLibraryNames
     * returns the list of component library file names without path and extension.
     *
     * @param aSorted - Sort the list of name if true.  Otherwise use the
     *                  library load order.
     * @return The list of library names.
     */
    static wxArrayString GetLibraryNames( bool aSorted = true );

    /**
     * Function FindLibraryComponent
     * searches all libraries in the list for a component.
     *
     * A component object will always be returned.  If the entry found
     * is an alias.  The root component will be found and returned.
     *
     * @param aComponentName - Name of component to search for.
     * @param aLibraryName - Name of the library to search for component.
     * @return The component object if found, otherwise NULL.
     */
    static LIB_COMPONENT* FindLibraryComponent( const wxString& aComponentName,
                                                const wxString& aLibraryName = wxEmptyString );

    /**
     * Function FindLibraryEntry
     * searches all libraries in the list for an entry.
     *
     * The object can be either a component or an alias.
     *
     * @param aEntryName - Name of entry to search for.
     * @param aLibraryName - Name of the library to search.
     * @return The entry object if found, otherwise NULL.
     */
    static LIB_ALIAS* FindLibraryEntry( const wxString& aEntryName,
                                        const wxString& aLibraryName = wxEmptyString );

    /**
     * Function RemoveCacheLibrary
     * removes all cache libraries from library list.
     */
    static void RemoveCacheLibrary();

    static int GetLibraryCount() { return libraryList.size(); }

    static CMP_LIBRARY_LIST& GetLibraryList()
    {
        return libraryList;
    }

    static void SetSortOrder( const wxArrayString& aSortOrder )
    {
        libraryListSortOrder = aSortOrder;
    }

    static wxArrayString& GetSortOrder( void )
    {
        return libraryListSortOrder;
    }
};


/**
 * Case insensitive library name comparison.
 */
extern bool operator==( const CMP_LIBRARY& aLibrary, const wxChar* aName );
extern bool operator!=( const CMP_LIBRARY& aLibrary, const wxChar* aName );

#endif  //  CLASS_LIBRARY_H
