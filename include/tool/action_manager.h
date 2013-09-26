/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#ifndef ACTION_MANAGER_H_
#define ACTION_MANAGER_H_

#include <map>
#include <string>

class TOOL_BASE;
class TOOL_MANAGER;
class TOOL_ACTION;

class ACTION_MANAGER
{
public:
    /**
     * Constructor.
     * @param aToolManager is a tool manager instance that is used to pass events to tools.
     */
    ACTION_MANAGER( TOOL_MANAGER* aToolManager );

    /**
     * Function RegisterAction()
     * Adds a tool action to the manager set and sets it up. After that is is possible to invoke
     * the action using hotkeys or its name.
     * @param aAction: action to be added. Ownership is not transferred.
     */
    void RegisterAction( TOOL_ACTION* aAction );

    /**
     * Function UnregisterAction()
     * Removes a tool action from the manager set and makes it unavailable for further usage.
     * @param aAction: action to be removed.
     */
    void UnregisterAction( TOOL_ACTION* aAction );

    /**
     * Generates an unique ID from for a tool with given name.
     */
    static int MakeActionId( const std::string& aActionName );

    /**
     * Function RunAction()
     * Runs an action with a given name (if there is one available).
     * @param aActionName is the name of action to be run.
     * @return True if there was an action associated with the name, false otherwise.
     */
    bool RunAction( const std::string& aActionName ) const;

    // TODO to be considered
    //bool RunAction( int aActionId ) const;
    //bool RunAction( TOOL_ACTION* aAction ) const;

    /**
     * Function RunHotKey()
     * Runs an action associated with a hotkey (if there is one available).
     * @param aHotKey is the hotkey to be served.
     * @return True if there was an action associated with the hotkey, false otherwise.
     */
    bool RunHotKey( int aHotKey ) const;

private:
    TOOL_MANAGER* m_toolMgr;
    std::map<int, TOOL_ACTION*> m_actionIdIndex;
    std::map<std::string, TOOL_ACTION*> m_actionNameIndex;
    std::map<int, TOOL_ACTION*> m_actionHotKeys;

    /**
     * Function runAction()
     * Prepares an appropriate event and sends it to the destination specified in a TOOL_ACTION
     * object.
     * @param aAction is the action to be run.
     */
    void runAction( const TOOL_ACTION* aAction ) const;
};

#endif /* ACTION_MANAGER_H_ */
