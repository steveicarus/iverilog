#ifndef IVL_PPackage_H
#define IVL_PPackage_H
/*
 * Copyright (c) 2012-2014 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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

# include  "PScope.h"
# include  "LineInfo.h"
# include  "StringHeap.h"
# include  <iostream>
# include  <vector>

/*
 * SystemVerilog supports class declarations with their own lexical
 * scope, etc. The parser arranges for these to be created and
 * collected.
 */

class PPackage : public PScopeExtra, public LineInfo {

    public:
      explicit PPackage (perm_string name, LexicalScope*parent);
      ~PPackage();

      bool elaborate_scope(Design*des, NetScope*scope);
      bool elaborate_sig(Design*des, NetScope*scope) const;
      bool elaborate(Design*des, NetScope*scope) const;

      void pform_dump(std::ostream&out) const;

      struct export_t {
	    PPackage *pkg;
	    perm_string name;
      };

      std::vector<export_t> exports;
};

#endif /* IVL_PPackage_H */
