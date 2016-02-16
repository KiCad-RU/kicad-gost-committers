/**
 * @file board_netlist_updater.h
 * @brief BOARD_NETLIST_UPDATER class definition
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2015 CERN
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
 *
 * Copyright (C) 1992-2015 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef __BOARD_NETLIST_UPDATER_H
#define __BOARD_NETLIST_UPDATER_H

class BOARD;
class REPORTER;
class NETLIST;
class COMPONENT;
class MODULE;
class PICKED_ITEMS_LIST;
class PCB_EDIT_FRAME;

#include <class_undoredo_container.h>

/**
 * Class BOARD_NETLIST_UPDATER
 * updates the #BOARD with a new netlist.
 *
 * The changes are made to the board are as follows they are not disabled in the status
 * settings in the #NETLIST:
 * - If a new component is found in the #NETLIST and not in the #BOARD, it is added
 *   to the #BOARD.
 * - If a the component in the #NETLIST is already on the #BOARD, then one or more of the
 *   following actions can occur:
 *   + If the footprint name in the #NETLIST does not match the footprint name on the
 *     #BOARD, the footprint on the #BOARD is replaced with the footprint specified in
 *     the #NETLIST and the proper parameters are copied from the existing footprint.
 *   + If the reference designator in the #NETLIST does not match the reference designator
 *     on the #BOARD, the reference designator is updated from the #NETLIST.
 *   + If the value field in the #NETLIST does not match the value field on the #BOARD,
 *     the value field is updated from the #NETLIST.
 *   + If the time stamp in the #NETLIST does not match the time stamp  on the #BOARD,
 *     the time stamp is updated from the #NETLIST.
 * - After each footprint is added or update as described above, each footprint pad net
 *   name is compared and updated to the value defined in the #NETLIST.
 * - After all of the footprints have been added, updated, and net names properly set,
 *   any extra unlock footprints are removed from the #BOARD.
 *
 */
class BOARD_NETLIST_UPDATER
{
public:

	BOARD_NETLIST_UPDATER( PCB_EDIT_FRAME *aFrame, BOARD *aBoard );
	~BOARD_NETLIST_UPDATER();

	/**
	 * Function UpdateNetlist()
	 *
	 * Updates the board's components according to the new netlist.
	 * See BOARD_NETLIST_UPDATER class description for the details of the process.
	 * @param aNetlist the new netlist
	 * @return true if process was completed successfully
	 */
	bool UpdateNetlist( NETLIST& aNetlist );

	// @todo: implement and move NETLIST::ReadPcbNetlist here
	bool UpdateNetlist( const wxString& aNetlistFileName,
                        const wxString& aCmpFileName );


	///> Sets the reporter object
	void SetReporter ( REPORTER *aReporter )
	{
		m_reporter = aReporter;
	}

	///> Enables "delete single pad nets" option
	void SetDeleteSinglePadNets( bool aEnabled )
	{
		m_deleteSinglePadNets = aEnabled;
	}

	///> Enables dry run mode (just report, no changes to PCB)
	void SetIsDryRun ( bool aEnabled )
	{
		m_isDryRun = aEnabled;
	}

	///> Enables replacing footprints with new ones
	void SetReplaceFootprints ( bool aEnabled )
	{
		m_replaceFootprints = aEnabled;
	}

	///> Enables removing unused components
	void SetDeleteUnusedComponents ( bool aEnabled )
	{
		m_deleteUnusedComponents = aEnabled;
	}

	///> Enables component lookup by timestamp instead of reference
	void SetLookupByTimestamp ( bool aEnabled )
	{
		m_lookupByTimestamp = aEnabled;
	}

	std::vector<MODULE*> GetAddedComponents() const
	{
            return m_addedComponents;
	}

private:

	void pushUndo( BOARD_ITEM* aItem, UNDO_REDO_T aCommandType, BOARD_ITEM* aCopy = NULL );

	wxPoint estimateComponentInsertionPosition();
	MODULE* addNewComponent( COMPONENT* aComponent );
	MODULE* replaceComponent( NETLIST& aNetlist, MODULE *aPcbComponent, COMPONENT* aNewComponent );
	bool updateComponentParameters( MODULE *aPcbComponent, COMPONENT* aNewComponent );
	bool updateComponentPadConnections( MODULE *aPcbComponent, COMPONENT* aNewComponent );
	bool deleteUnusedComponents( NETLIST& aNetlist );
	bool deleteSinglePadNets();
	bool testConnectivity( NETLIST& aNetlist );

	PICKED_ITEMS_LIST *m_undoList;
	PCB_EDIT_FRAME *m_frame;
	BOARD *m_board;
	REPORTER *m_reporter;

	std::vector<MODULE*> m_addedComponents;

	bool m_deleteSinglePadNets;
	bool m_deleteUnusedComponents;
	bool m_isDryRun;
	bool m_replaceFootprints;
	bool m_lookupByTimestamp;

	int m_warningCount;
	int m_errorCount;
};

#endif
