/*
 * Copyright (C) 2016 Mark Roszko <mark.roszko@gmail.com>
 * Copyright (C) 2016 QiEDA Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ISEXPRABLE_H_
#define ISEXPRABLE_H_

#include "sexpr/sexpr.h"

namespace SEXPR
{
    class SEXPR;
    class SEXPR_LIST;

    class ISEXPRABLE
    {
    public:
        virtual ~ISEXPRABLE() {}
        virtual SEXPR* SerializeSEXPR() const { return nullptr; }
        virtual void DeserializeSEXPR(SEXPR& sexp) {}
    };
}

#endif
