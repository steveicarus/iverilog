#ifndef IVL_PClass_H
#define IVL_PClass_H
/*
 * Copyright (c) 2012-2026 Stephen Williams (steve@icarus.com)
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
# include  "PNamedItem.h"
# include  "StringHeap.h"
# include  <iostream>

class PChainConstructor;

/*
 * SystemVerilog supports class declarations with their own lexical
 * scope, etc. The parser arranges for these to be created and
 * collected.
 */

class PClass : public PScopeExtra, public PNamedItem {

    public:
      explicit PClass (perm_string name, LexicalScope*parent);
      ~PClass() override;

      void dump(std::ostream&out, unsigned indent) const;

      SymbolType symbol_type() const override;

    public:
      class_type_t*type;

	// Methods declared "extern" in the class body. The definition
	// is given outside the class (IEEE1800-2017 8.24).
      struct extern_method_t : public LineInfo {
	    bool is_task;
	    bool is_virtual;
	    bool defined;
	    std::vector<pform_tf_port_t>*ports;
      };
      std::map<perm_string,extern_method_t> externs;
};

#endif /* IVL_PClass_H */
