#!/usr/bin/env python

# This program source code file is part of KiCad, a free EDA CAD application.
#
# Copyright (C) 2013 Alexander Lunev <al.lunev@yahoo.com>
# Copyright (C) 2013 KiCad Developers, see AUTHORS.txt for contributors.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, you may find one here:
# http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
# or you may search the http://www.gnu.org website for the version 2 license,
# or you may write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


import uno, time

class UNO_IFACE():

    def __init__( self ):
        self.desktop = None
        self.document = None
        self.curTable = None

    def Connect( self ):
        local = uno.getComponentContext()
        resolver = local.ServiceManager.createInstanceWithContext( "com.sun.star.bridge.UnoUrlResolver", local )

        connected = False
        for retry in range( 10 ):
            try:
                context = resolver.resolve( "uno:socket,host=localhost,port=8100;urp;StarOffice.ComponentContext" )
                connected = True
                break
            except:
                time.sleep( 1 )

        if( not connected ):
            return

        self.desktop = context.ServiceManager.createInstanceWithContext( "com.sun.star.frame.Desktop", context )


    def LoadDocument( self, aUrl ):
        self.document = self.desktop.loadComponentFromURL( aUrl, "_blank", 0, () )
        self.SelectTable( 0 )


    def AppendDocument( self, aUrl ):
        textRange = self.document.Text.End
        cursor = self.document.Text.createTextCursorByRange( textRange )
        cursor.insertDocumentFromURL( aUrl, () )


    def SelectTable( self, aIndex ):
        self.curTable = self.document.TextTables.getByIndex( aIndex )


    def PutCell( self, aCellAddress, aStr, aStyle ):
        cell = self.curTable.getCellByName( aCellAddress )
        cursor = cell.createTextCursor()

        if( aStyle != 0 ):
            cursor.setPropertyValue( "CharUnderline", 1 )
            cursor.setPropertyValue( "ParaAdjust", 3 )

        cursor.setString( aStr )


uno_iface_inst = UNO_IFACE()
