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


import sys, os, time
import uno
from com.sun.star.beans import PropertyValue
import socket
import re

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
            return False

        self.desktop = context.ServiceManager.createInstanceWithContext( "com.sun.star.frame.Desktop", context )

        return True


    def LoadDocument( self, aUrl ):
        try:
            hidden = PropertyValue( "Hidden" , 0 , True, 0 ),
            self.document = self.desktop.loadComponentFromURL( aUrl, "_blank", 0, (hidden) )
            self.SelectTable( 0 )
            return True
        except:
            return False


    def AppendDocument( self, aUrl ):
        try:
            textRange = self.document.Text.End
            cursor = self.document.Text.createTextCursorByRange( textRange )
            cursor.insertDocumentFromURL( aUrl, () )
            return True
        except:
            return False


    def SelectTable( self, aIndex ):
        try:
            self.curTable = self.document.TextTables.getByIndex( aIndex )
            return True
        except:
            return False


    def PutCell( self, aCellAddress, aStr, aStyle ):
        try:
            cell = self.curTable.getCellByName( aCellAddress )
            cursor = cell.createTextCursor()

            if( aStyle != 0 ):
                cursor.setPropertyValue( "CharUnderline", 1 )
                cursor.setPropertyValue( "ParaAdjust", 3 )

            cursor.setString( aStr )
            return True
        except:
            return False


    def MakeVisible( self ):
        try:
            self.document.CurrentController.Frame.ContainerWindow.Visible = True
            self.document.CurrentController.Frame.ContainerWindow.toFront()
            return True
        except:
            return False


dbg_ena = False
if( len( sys.argv ) > 1 and sys.argv[1]=='DBG=ON' ):
    dbg_ena = True
    # redirect stdout and stderr into logfile
    # put logfile into the current user's HOME directory
    logfile = os.path.join( os.path.expanduser( '~' ), 'kicad_uno_iface.log' )
    sys.stdout = open( logfile, 'w' )
    sys.stderr = open( logfile, 'w' )
    print( 'Python version:', sys.version_info )

uno_iface_inst = UNO_IFACE()

host = 'localhost'
port = 8101
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((host, port))
s.listen(1)
sock, addr = s.accept()
#print 'Connected with', addr

while True:
    buf = sock.recv( 1024 )

    if( dbg_ena ):
        # log RPC commands
        print( buf )

    buf = buf.decode( 'UTF-8' )

    # "Exit"
    args = re.findall( u'^Exit', buf )
    if( args != [] ):
        uno_iface_inst.MakeVisible()
        sock.send( b'BYE_____' )
        break

    # "Connect"
    args = re.findall( u'^Connect', buf )
    if( args != [] ):
        if( uno_iface_inst.Connect() ):
            sock.send( b'OK______' )
        else:
            sock.send( b'FAILED__' )
        continue

    # "LoadDocument {arg}"
    args = re.findall( u'^LoadDocument {(.*)}', buf )
    if( args != [] ):
        if( uno_iface_inst.LoadDocument( args[0] ) ):
            sock.send( b'OK______' )
        else:
            sock.send( b'FAILED__' )
        continue

    # "AppendDocument {arg}"
    args = re.findall( u'^AppendDocument {(.*)}', buf )
    if( args != [] ):
        if( uno_iface_inst.AppendDocument( args[0] ) ):
            sock.send( b'OK______' )
        else:
            sock.send( b'FAILED__' )
        continue

    # "SelectTable {arg}"
    args = re.findall( u'^SelectTable {(.*)}', buf )
    if( args != [] ):
        if( uno_iface_inst.SelectTable( int( args[0] ) ) ):
            sock.send( b'OK______' )
        else:
            sock.send( b'FAILED__' )
        continue

    # "PutCell {arg} {arg} {arg}"
    args = re.findall( u'^PutCell {(.*)} {(.*)} {(.*)}', buf, re.DOTALL )
    if( args != [] and args[0] != [] ):
        if( uno_iface_inst.PutCell( args[0][0], args[0][1], int( args[0][2] ) ) ):
            sock.send( b'OK______' )
        else:
            sock.send( b'FAILED__' )
        continue

    if( dbg_ena ):
        print( 'error: received unknown command' )

    sock.send( b'UNKWNCMD' )

sock.close()
s.close()
