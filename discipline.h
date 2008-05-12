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

class nature_t : public LineInfo {
    public:
      explicit nature_t(perm_string name, perm_string access);
      ~nature_t();

      perm_string name() const   { return name_; }
	// Identifier for the access function for this nature
      perm_string access() const { return access_; }

    private:
      perm_string name_;
      perm_string access_;
};

class discipline_t : public LineInfo {
    public:
      explicit discipline_t (perm_string name, ddomain_t dom,
			     nature_t*pot, nature_t*flow);
      ~discipline_t();

      perm_string name() const         { return name_; }
      ddomain_t domain() const         { return domain_; }
      const nature_t*potential() const { return potential_; }
      const nature_t*flow() const      { return flow_; }

    private:
      perm_string name_;
      ddomain_t domain_;
      nature_t*potential_;
      nature_t*flow_;

    private: // not implemented
      discipline_t(const discipline_t&);
      discipline_t& operator = (const discipline_t&);
};

extern map<perm_string,nature_t*> natures;
extern map<perm_string,discipline_t*> disciplines;

#endif
