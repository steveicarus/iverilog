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

#include "vhdlreal.h"
#include <assert.h>
#include <cstring>
#include <cstdlib>

using namespace std;

vhdlreal::vhdlreal() {
    value_ = 0.0;
}

vhdlreal::vhdlreal(const double& r) {
    value_ = r;
}

vhdlreal::vhdlreal(const vhdlreal& val) {
    value_ = val.as_double();
}

vhdlreal::vhdlreal(const char* text) {
    assert(strlen(text) != 0);
    char* buffer = new char[strlen(text)+1];

    char* buf_ptr;
    for(buf_ptr = buffer; *text != 0; ++buf_ptr, ++text) {
        if(*text == '_')
            continue;
        *buf_ptr = *text;
    }
    *buf_ptr = '\0';

    value_ = strtod(buffer, NULL);
    delete[] buffer;
}

ostream& operator<< (ostream& str, const vhdlreal& r) {
    return (str << r.as_double());
}
vhdlreal operator+ (const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(r1.as_double() + r2.as_double());
}
vhdlreal operator- (const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(r1.as_double() - r2.as_double());
}
vhdlreal operator* (const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(r1.as_double() * r2.as_double());
}
vhdlreal operator/ (const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(r1.as_double() / r2.as_double());
}
vhdlreal operator% (const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(fmod(r1.as_double(), r2.as_double()));
}
vhdlreal pow(const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(pow(r1.as_double(), r2.as_double()));
}
vhdlreal operator- (const vhdlreal& r) {
    return vhdlreal(-r.as_double());
}
