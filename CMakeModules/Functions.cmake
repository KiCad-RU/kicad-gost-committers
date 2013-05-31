#  This program source code file is part of KICAD, a free EDA CAD application.
#
#  Copyright (C) 2010 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
#  Copyright (C) 2010 Kicad Developers, see AUTHORS.txt for contributors.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, you may find one here:
#  http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
#  or you may search the http://www.gnu.org website for the version 2 license,
#  or you may write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
#


# Function make_lexer
# is a standard way to invoke TokenList2DsnLexer.cmake.
# Extra arguments are treated as source files which depend on the generated
# outHeaderFile

function( make_lexer inputFile outHeaderFile outCppFile enum )
    add_custom_command(
        OUTPUT  ${outHeaderFile}
                ${outCppFile}
        COMMAND ${CMAKE_COMMAND}
            -Denum=${enum}
            -DinputFile=${inputFile}
            -DoutHeaderFile=${outHeaderFile}
            -DoutCppFile=${outCppFile}
            -P ${CMAKE_MODULE_PATH}/TokenList2DsnLexer.cmake
        DEPENDS ${inputFile}
                ${CMAKE_MODULE_PATH}/TokenList2DsnLexer.cmake
        COMMENT "TokenList2DsnLexer.cmake creating:
           ${outHeaderFile} and
           ${outCppFile} from
           ${inputFile}"
        )

    # extra_args, if any, are treated as source files (typically headers) which
    # are known to depend on the generated outHeader.
    foreach( extra_arg ${ARGN} )
        set_source_files_properties( ${extra_arg}
            PROPERTIES OBJECT_DEPENDS ${outHeaderFile}
            )
    endforeach()

endfunction()

