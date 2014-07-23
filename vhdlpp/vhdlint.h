#ifndef IVL_vhdlint_H
#define IVL_vhdlint_H
/*
 * Copyright (c) 2011-2014 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdint.h>

class vhdlint
{
    public:
        explicit vhdlint(const char* text);
        explicit vhdlint(const int64_t& val);
        explicit vhdlint(const vhdlint& val);

        bool is_negative() const;
        bool is_positive() const;
        bool is_zero() const;

        int64_t as_long() const;
        //vhdlv get(const unsigned index) const;
        //void set(const unsigned index, const unsigned val);
       // unsigned short operator[](const unsigned index);
    private:
        int64_t value_;
};

#endif /* IVL_vhdlint_H */
