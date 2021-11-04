#ifndef IVL_discipline_H
#define IVL_discipline_H
/*
 * Copyright (c) 2008-2021 Stephen Williams (steve@icarus.com)
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

/*
 * The discipline types include the discipline, nature and other
 * related types for managing declared and used disciplines in a
 * Verilog-AMS program.
 */

# include  "StringHeap.h"
# include  <iostream>
# include  <map>
# include  "ivl_target.h"
# include  "LineInfo.h"

extern std::ostream& operator << (std::ostream&, ivl_dis_domain_t);

class ivl_nature_s : public LineInfo {
    public:
      explicit ivl_nature_s(perm_string name, perm_string access);
      ~ivl_nature_s();

      perm_string name() const   { return name_; }
	// Identifier for the access function for this nature
      perm_string access() const { return access_; }

    private:
      perm_string name_;
      perm_string access_;
};

class ivl_discipline_s : public LineInfo {
    public:
      explicit ivl_discipline_s (perm_string name, ivl_dis_domain_t dom,
				 ivl_nature_t pot, ivl_nature_t flow);
      ~ivl_discipline_s();

      perm_string name() const         { return name_; }
      ivl_dis_domain_t domain() const  { return domain_; }
      ivl_nature_t potential() const { return potential_; }
      ivl_nature_t flow() const      { return flow_; }

    private:
      perm_string name_;
      ivl_dis_domain_t domain_;
      ivl_nature_t potential_;
      ivl_nature_t flow_;

    private: // not implemented
      ivl_discipline_s(const ivl_discipline_s&);
      ivl_discipline_s& operator = (const ivl_discipline_s&);
};

extern std::map<perm_string,ivl_nature_t> natures;
extern std::map<perm_string,ivl_discipline_t> disciplines;
  // Map access function name to the nature that it accesses.
extern std::map<perm_string,ivl_nature_t> access_function_nature;

#endif /* IVL_discipline_H */
