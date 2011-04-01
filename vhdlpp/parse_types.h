#ifndef __parse_types_H
#define __parse_types_H
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "StringHeap.h"
class Expression;

class named_expr_t {

    public:
      named_expr_t (perm_string n, Expression*e) : name_(n), expr_(e) { }

      perm_string name() const { return name_; }
      Expression* expr() const { return expr_; }
    private:
      perm_string name_;
      Expression* expr_;

    private: // Not implemented
      named_expr_t(const named_expr_t&);
      named_expr_t& operator = (const named_expr_t&);
};

#endif
