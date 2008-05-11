#ifndef __discipline_H
#define __discipline_H
/*
 * Copyright (c) 2008 Stephen Williams (steve@icarus.com)
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

/*
 * The discipline types include the discipline, nature and other
 * related types for managing declared and used disciplines in a
 * Verilog-AMS program.
 */

# include  "StringHeap.h"
# include  <iostream>
# include  <map>
# include  "LineInfo.h"

typedef enum { DD_NONE, DD_DISCRETE, DD_CONTINUOUS } ddomain_t;
extern std::ostream& operator << (std::ostream&, ddomain_t);

class discipline_t : public LineInfo {
    public:
      explicit discipline_t (perm_string name, ddomain_t dom);
      ~discipline_t();

      perm_string name() const { return name_; }
      ddomain_t domain() const { return domain_; }

    private:
      perm_string name_;
      ddomain_t domain_;
};

extern map<perm_string,discipline_t*> disciplines;

#endif
