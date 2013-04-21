/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007-2013 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2013 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file dictionaries.cpp
 */

#include <dictionaries.h>

namespace GOST_DOC_GEN {

TDICTIONARY_ITEM dictionary[DICTIONARY_SIZE] =
{
    {
        wxT( "Диодная сборка" ),
        wxT( "Диодные сборки" ),
        wxT( "Диодной сборки" ),
        wxT( "Диодных сборок" )
    },
    {
        wxT( "Дроссель" ),
        wxT( "Дроссели" ),
        wxT( "Дросселя" ),
        wxT( "Дросселей" )
    },
    {
        wxT( "Микросхема" ),
        wxT( "Микросхемы" ),
        wxT( "Микросхемы" ),
        wxT( "Микросхем" )
    },
    {
        wxT( "Кабель" ),
        wxT( "Кабели" ),
        wxT( "Кабеля" ),
        wxT( "Кабелей" )
    },
    {
        wxT( "Кварцевый резонатор" ),
        wxT( "Кварцевые резонаторы" ),
        wxT( "Кварцевого резонатора" ),
        wxT( "Кварцевых резонасторов" )
    },
    {
        wxT( "Кнопка" ),
        wxT( "Кнопки" ),
        wxT( "Кнопки" ),
        wxT( "Кнопок" )
    },
    {
        wxT( "Колодка зажимов" ),
        wxT( "Колодки зажимов" ),
        wxT( "Колодки зажимов" ),
        wxT( "Колодок зажимов" )
    },
    {
        wxT( "Крепление для элемента питания" ),
        wxT( "Крепления для элементов питания" ),
        wxT( "Крепления для элемента питания" ),
        wxT( "Креплений для элементов питания" )
    },
    {
        wxT( "Оптопара" ),
        wxT( "Оптопары" ),
        wxT( "Оптопары" ),
        wxT( "Оптопар" )
    },
    {
        wxT( "Переключатель" ),
        wxT( "Переключатели" ),
        wxT( "Переключателя" ),
        wxT( "Переключателей" )
    },
    {
        wxT( "Предохранитель" ),
        wxT( "Предохранители" ),
        wxT( "Предохранителя" ),
        wxT( "Предохранителей" )
    },
    {
        wxT( "Планка" ),
        wxT( "Планки" ),
        wxT( "Планки" ),
        wxT( "Планок" )
    },
    {
        wxT( "Резисторная сборка" ),
        wxT( "Резисторные сборки" ),
        wxT( "Резисторной сборки" ),
        wxT( "Резисторных сборок" )
    },
    {
        wxT( "Реле" ),
        wxT( "Реле" ),
        wxT( "Реле" ),
        wxT( "Реле" )
    },
    {
        wxT( "Стойка" ),
        wxT( "Стойки" ),
        wxT( "Стойки" ),
        wxT( "Стоек" )
    },
    {
        wxT( "Элемент защиты" ),
        wxT( "Элементы защиты" ),
        wxT( "Элемента защиты" ),
        wxT( "Элементов защиты" )
    },
};

TMEASURE_UNIT meas_units_matrix[UNITS_MATRIX_SIZE] =
{
    { wxT( "мкГн" ), -6 },
    { wxT( "кОм" ),   3 },
    { wxT( "МОм" ),   6 },
    { wxT( "мкФ" ),  -6 },
    { wxT( "нГн" ),  -9 },
    { wxT( "кГц" ),   3 },
    { wxT( "МГц" ),   6 },
    { wxT( "мГн" ),  -3 },
    { wxT( "Ом" ),    0 },
    { wxT( "пФ" ),  -12 },
    { wxT( "Гц" ),    0 },
    { wxT( "нФ" ),   -9 },
    { wxT( "мк" ),   -6 },
    { wxT( "Гн" ),    0 },
    { wxT( "к" ),     3 },
    { wxT( "н" ),    -9 },
    { wxT( "М" ),     6 }
};

} // namespace GOST_DOC_GEN
