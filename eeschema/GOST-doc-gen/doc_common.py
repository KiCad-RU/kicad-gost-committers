#!/usr/bin/env python
# -*- coding: utf-8 -*-
### BEGIN LICENSE
# This program source code file is part of KiCad, a free EDA CAD application.
#
# Copyright (C) 2007-2013 Alexander Lunev <al.lunev@yahoo.com>
# Copyright (C) 2013 Baranovskiy Konstantin <baranvoskiykonstantin@gmail.com>
# Copyright (C) 2013-2015 KiCad Developers, see AUTHORS.txt for contributors.
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
### END LICENSE

import re

FIRST_SHEET_LAST_STR_I = 26
SECOND_SHEET_LAST_STR_I = 29

WORDFORM_PLURAL = 0
WORDFORM_SINGULAR_GENITIVE = 1
WORDFORM_PLURAL_GENITIVE = 2

SPEC_NO_SORTING = 1
SPEC_GEN_POS_LIST = 2

NAME_LENGTH_MAX = 46
NOTE_LENGTH_MAX = 20

SPEC_NAME_LENGTH_MAX = 28
SPEC_NOTE_LENGTH_MAX = 10

ADDR_CMPIDX_COL_REFDES = '1:A'
ADDR_CMPIDX_COL_NAME = '1:B'
ADDR_CMPIDX_COL_QTY = '1:C'
ADDR_CMPIDX_COL_NOTE = '1:D'

ADDR_SPEC_COL_FORMAT = '1:A'
ADDR_SPEC_COL_POSITION = '1:C'
ADDR_SPEC_COL_DESIGNATION = '1:D'
ADDR_SPEC_COL_NAME = '1:E'
ADDR_SPEC_COL_QTY = '1:F'
ADDR_SPEC_COL_NOTE = '1:G'

ADDR_DESIGN_NAME = '3:E4'
ADDR_DESIGNATION = '3:F1'
ADDR_FIRST_USE = '0:B1'
ADDR_DEVELOPER = '3:B4'
ADDR_VERIFIER = '3:B5'
ADDR_APPROVER = '3:B8'
ADDR_COMPANY = '3:F6'
ADDR_SHEET_QTY = '3:J5'

ADDR_MIDSHEET_NUMBER = '3:G3'
ADDR_MIDSHEET_DESIGNATION = '3:F1'

ADDR_LASTSHEET_NUMBER = '3:I5'
ADDR_LASTSHEET_DESIGNATION = '3:F1'


class DocCommon:
    """
    Common functions for working with the document.

    """

    def __init__(self):
        """
        Initialization of the DocCommon fields.

        """
        self.current_row = 0
        self.current_sheet = 0
        self.specification_pos_field = 0
        self.dictionary = {
            u'Диодная сборка': (
                u'Диодные сборки',
                u'Диодной сборки',
                u'Диодных сборок'
                ),
            u'Микросхема': (
                u'Микросхемы',
                u'Микросхемы',
                u'Микросхем'
                ),
            u'Кварцевый резонатор': (
                u'Кварцевые резонаторы',
                u'Кварцевого резонатора',
                u'Кварцевых резонаторов'
                ),
            u'Кнопка': (
                u'Кнопки',
                u'Кнопки',
                u'Кнопок'
                ),
            u'Колодка зажимов': (
                u'Колодки зажимов',
                u'Колодки зажимов',
                u'Колодок зажимов'
                ),
            u'Крепление для элемента питания': (
                u'Крепления для элементов питания',
                u'Крепления для элемента питания',
                u'Креплений для элементов питания'
                ),
            u'Оптопара': (
                u'Оптопары',
                u'Оптопары',
                u'Оптопар'
                ),
            u'Планка': (
                u'Планки',
                u'Планки',
                u'Планок'
                ),
            u'Резисторная сборка': (
                u'Резисторные сборки',
                u'Резисторной сборки',
                u'Резисторных сборок'
                ),
            u'Реле': (
                u'Реле',
                u'Реле',
                u'Реле'
                ),
            u'Стойка': (
                u'Стойки',
                u'Стойки',
                u'Стоек'
                ),
            u'Элемент защиты': (
                u'Элементы защиты',
                u'Элемента защиты',
                u'Элементов защиты'
                )
            }

        self.meas_units_matrix = (
            (u'мкГн', -6),
            (u'кОм', 3),
            (u'МОм', 6),
            (u'ГОм', 9),
            (u'мкФ', -6),
            (u'нГн', -9),
            (u'кГц', 3),
            (u'МГц', 6),
            (u'ГГц', 9),
            (u'мГн', -3),
            (u'Ом', 0),
            (u'пФ', -12),
            (u'Гц', 0),
            (u'мФ', -3),
            (u'нФ', -9),
            (u'мк', -6),
            (u'Гн', 0),
            (u'Ф', 0),
            (u'к', 3),
            (u'н', -9),
            (u'М', 6),
            (u'Г', 9)
            )

    def change_word_form(self, str_, type_):
        """
        Change form of the word.

            Arguments:

            str_ - word or collocation of word in singular form;
            type_ - type of the form:
                0 - plural;
                1 - singular genitive;
                2 - plural genitive.

            Returns:

            output - word or collocation of word changed to the specified type.

        """
        output = None
        if str_ in self.dictionary:
            output = self.dictionary[str_][type_]
        else:
            last_char = str_[-1:]
            if last_char == u'ь':
                output = str_[:-1] + (u'и', u'я', u'ей')[type_]
            else:
                output = str_ + (u'ы', u'а', u'ов')[type_]
        return output

    def extract_absolute_value(self, value):
        """
        Get absolute value of component from string.

            Arguments:

            value - string with value of component like '100 кОм', etc.

            Returns:

            if value is correct - absolute value;
            else - None.

        """
        value = value.replace(u',', u'.')
        match = re.search(r'\D\.?\d+(?:\.\d+)?', value)
        if match:
            mant = float(match.group(0))
            for unit, exp in self.meas_units_matrix:
                if unit in value:
                    return mant * (10 ** exp)
            return mant
        else:
            return None

    def sort_string_array(self, string_array):
        """
        Sortsing an array of strings.

            Arguments:

            string_array - array of strings.

        """
        for i in range(len(string_array)):
            # compare two neighbour strings
            for j in range(len(string_array) - i):
                if string_array[j] > string_array[j + 1]:
                    # if the strings are placed out of order
                    # then the string are swapped
                    string_array[j], string_array[j + 1] = string_array[j + 1], string_array[j]
