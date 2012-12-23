
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

#include "vhdlint.h"
#include <cstring>
#include <cstdlib>
#include <sstream>

using namespace std;

bool vhdlint::is_negative() const
{
    return value_ < 0L;
}

bool vhdlint::is_positive() const
{
    return value_ > 0L;
}

bool vhdlint::is_zero() const
{
    return value_ == 0L;
}

vhdlint::vhdlint(const char* text)
{
    unsigned text_length = strlen(text);
    if(text_length == 0)
    {
        value_ = 0L;
        return;
    }

    char* new_text = new char[text_length + 1];

    const char* ptr;
    char* new_ptr;
    for(ptr = text, new_ptr = new_text; *ptr != 0; ++ptr)
    {
        if(*ptr == '_')
            continue;
        else
        {
            *new_ptr = *ptr;
            ++new_ptr;
        }
    }
    *new_ptr = 0;

    istringstream str(new_text);
    delete[] new_text;

    //TODO: check if numbers greater than MAX_INT are handled correctly
    str >> value_;
}

vhdlint::vhdlint(const int64_t& val)
{
    value_ = val;
}

vhdlint::vhdlint(const vhdlint& val)
{
    value_ = val.as_long();
}

int64_t vhdlint::as_long() const
{
    return value_;
}
