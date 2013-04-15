#ifndef __netclass_H
#define __netclass_H
/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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

# include  "LineInfo.h"
# include  "ivl_target.h"
# include  "nettypes.h"
# include  <iostream>
# include  <map>

class Design;
class NetScope;
class PClass;

class netclass_t : public ivl_type_s {
    public:
      netclass_t(perm_string class_name);
      ~netclass_t();

	// Set the property of the class during elaboration. Set the
	// name and type, and return true. If the name is already
	// present, then return false.
      bool set_property(perm_string pname, ivl_type_s*ptype);

	// Set the scope for the class. The scope has no parents and
	// is used for the elaboration of methods (tasks/functions).
      void set_class_scope(NetScope*cscope);

	// As an ivl_type_s object, the netclass is always an
	// ivl_VT_CLASS object.
      ivl_variable_type_t base_type() const;

	// This is the name of the class type
      inline perm_string get_name() const { return name_; }

      const ivl_type_s* get_property(perm_string pname) const;

      inline size_t get_properties(void) const { return properties_.size(); }
      const char*get_prop_name(size_t idx) const;
      ivl_type_t get_prop_type(size_t idx) const;

      int property_idx_from_name(perm_string pname) const;

	// The task method scopes from the method name.
      NetScope*method_from_name(perm_string mname) const;

      void elaborate_sig(Design*des, PClass*pclass);
      void elaborate(Design*des, PClass*pclass);

      void emit_scope(struct target_t*tgt) const;

      void dump_scope(ostream&fd) const;

    private:
      perm_string name_;
	// Map properrty names to property table index.
      std::map<perm_string,size_t> properties_;
	// Vector of properties.
      struct prop_t {
	    perm_string name;
	    ivl_type_s* type;
      };
      std::vector<prop_t> property_table_;

	// This holds task/function definitions for methods.
      NetScope*class_scope_;
};

#endif
