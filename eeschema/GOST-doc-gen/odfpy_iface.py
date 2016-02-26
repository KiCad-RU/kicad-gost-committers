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


#import os
import odf.opendocument
from odf.text import P
from odf.table import *
from odf.style import Style, ParagraphProperties, TextProperties

class ODFPY_IFACE():

    def __init__( self ):
        self.document = None
        self.curTable = None


    def LoadDocument( self, aUrl ):
        self.document = odf.opendocument.load( aUrl )
        #SelectTable( 0 )
        self.document.save('/home/a-lunev/git/bzr/GOST-doc-gen/eeschema/GOST-doc-gen/templates/other.odt')


    def AppendDocument( self, aUrl ):
        anotherDoc = odf.opendocument.load( aUrl )

        # Copy all properties from anotherDoc to the document
        for meta in anotherDoc.meta.childNodes[:]:
            self.document.meta.addElement( meta )

        for font in anotherDoc.fontfacedecls.childNodes[:]:
            self.document.fontfacedecls.addElement(font)

        for style in anotherDoc.styles.childNodes[:]:
            self.document.styles.addElement( style )

        for masterstyle in anotherDoc.masterstyles.childNodes[:]:
            self.document.masterstyles.addElement( masterstyle )

        for autostyle in anotherDoc.automaticstyles.childNodes[:]:
            self.document.automaticstyles.addElement( autostyle )

        for setting in anotherDoc.settings.childNodes[:]:
            self.document.settings.addElement( setting )

        self.document.text.addElement( anotherDoc.text.getElementsByType( Table )[0] )

        self.document.save('/home/a-lunev/git/bzr/GOST-doc-gen/eeschema/GOST-doc-gen/templates/other.odt')


    def SelectTable( self, aIndex ):
        self.curTable = self.document.text.getElementsByType( Table )[aIndex]


    def PutCell( self, aCellAddress, aStr, aStyle ):
        print aCellAddress, aStr
        #col_i = ord( aCellAddress[0] ) - ord( 'A' )
        #print col_i
        #row_i = int( aCellAddress[1:] )
        #print row_i
        col_i = 2
        row_i = 27
        row = self.curTable.getElementsByType( TableRow )[row_i]
        cell = row.getElementsByType( TableCell )[col_i]
        textFound = False
        for p in cell.getElementsByType( P ):
            for p_data in p.childNodes:
                if p_data.tagName == 'Text':
                    textFound = True
                    p_data.data = '123' #aStr #text.decode('string_escape')

        if( not textFound ):
            p.addText( '123' )

        self.document.save('/home/a-lunev/git/bzr/GOST-doc-gen/eeschema/GOST-doc-gen/templates/other.odt')


odfpy_iface_inst = ODFPY_IFACE()
