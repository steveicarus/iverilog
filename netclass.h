#ifndef IVL_netclass_H
#define IVL_netclass_H
/*
 * Copyright (c) 2012-2021 Stephen Williams (steve@icarus.com)
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
# include  "property_qual.h"
# include  <iostream>
# include  <map>

class Design;
class NetExpr;
class NetNet;
class NetScope;
class PClass;
class PExpr;

class netclass_t : public ivl_type_s {
    public:
      netclass_t(perm_string class_name, const netclass_t*super);
      ~netclass_t();

	// Set the property of the class during elaboration. Set the
	// name and type, and return true. If the name is already
	// present, then return false.
      bool set_property(perm_string pname, property_qualifier_t qual, ivl_type_t ptype);

	// Set the scope for the class. The scope has no parents and
	// is used for the elaboration of methods
	// (tasks/functions). In other words, this is the class itself.
      void set_class_scope(NetScope*cscope);

      inline const NetScope* class_scope(void) const { return class_scope_; }

	// Set the scope for the class definition. This is the scope
	// where the class definition was encountered, and may be used
	// to locate symbols that the class definition may inherit
	// from its context. This can be nil, or a package or module
	// where a class is defined.
      void set_definition_scope(NetScope*dscope);

      NetScope*definition_scope(void);

	// As an ivl_type_s object, the netclass is always an
	// ivl_VT_CLASS object.
      ivl_variable_type_t base_type() const;

	// This is the name of the class type
      inline perm_string get_name() const { return name_; }

	// If this is derived from another class, then this method
	// returns a pointer to the super-class.
      inline const netclass_t* get_super() const { return super_; }

	// Get the number of properties in this class. Include
	// properties in the parent class.
      size_t get_properties(void) const;
	// Get information about each property.
      const char*get_prop_name(size_t idx) const;
      property_qualifier_t get_prop_qual(size_t idx) const;
      ivl_type_t get_prop_type(size_t idx) const;

	// These methods are used by the elaborator to note the
	// initializer for constant properties. Properties start out
	// as not initialized, and when elaboration detects an
	// assignment to the property, it is marked initialized.
      bool get_prop_initialized(size_t idx) const;
      void set_prop_initialized(size_t idx) const;

      bool test_for_missing_initializers(void) const;

	// Map the name of a property to its index. Return <0 if the
	// name is not a property in the class.
      int property_idx_from_name(perm_string pname) const;

	// The task method scopes from the method name.
      NetScope*method_from_name(perm_string mname) const;

	// Returns the constructor task method of the class. Might be nullptr if
	// there is nothing to do in the constructor.
      NetScope* get_constructor() const;

	// Find the elaborated signal (NetNet) for a static
	// property. Search by name. The signal is created by the
	// elaborate_sig pass.
      NetNet*find_static_property(perm_string name) const;

	// Test if this scope is a method within the class. This is
	// used to check scope for handling data protection keywords
	// "local" and "protected".
      bool test_scope_is_method(const NetScope*scope) const;

      void elaborate_sig(Design*des, PClass*pclass);
      void elaborate(Design*des, PClass*pclass);

      void emit_scope(struct target_t*tgt) const;
      bool emit_defs(struct target_t*tgt) const;

      std::ostream& debug_dump(std::ostream&fd) const;
      void dump_scope(std::ostream&fd) const;

      const NetExpr* get_parameter(Design *des, perm_string name,
				   ivl_type_t &par_type) const;

      void set_virtual(bool virtual_class) { virtual_class_ = virtual_class; }
      bool is_virtual() const { return virtual_class_; }

    protected:
      bool test_compatibility(ivl_type_t that) const;

    private:
      perm_string name_;
	// If this is derived from another base class, point to it
	// here.
      const netclass_t*super_;
	// Map property names to property table index.
      std::map<perm_string,size_t> properties_;
	// Vector of properties.
      struct prop_t {
	    perm_string name;
	    property_qualifier_t qual;
	    ivl_type_t type;
	    mutable bool initialized_flag;
      };
      std::vector<prop_t> property_table_;

	// This holds task/function definitions for methods.
      NetScope*class_scope_;

	// This holds the context for the class type definition.
      NetScope*definition_scope_;

      bool virtual_class_;
};

inline NetScope*netclass_t::definition_scope(void)
{
      return definition_scope_;
}

#endif /* IVL_netclass_H */
