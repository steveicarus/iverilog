#ifndef IVL_class_type_H
#define IVL_class_type_H
/*
 * Copyright (c) 2012-2014 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version
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

# include  <string>
# include  <vector>
# include  "vpi_priv.h"

class class_property_t;
class vvp_vector4_t;

/*
 * This represents the TYPE information for a class. A %new operator
 * uses this information to figure out how to construct an actual
 * instance.
 */
class class_type : public __vpiHandle {

    public:
      struct inst_x;
      typedef inst_x*inst_t;

    public:
      explicit class_type(const std::string&nam, size_t nprop);
      ~class_type();

	// This is the name of the class type.
      inline const std::string&class_name(void) const { return class_name_; }
	// Number of properties in the class definition.
      inline size_t property_count(void) const { return properties_.size(); }

	// Set the details about the property. This is used during
	// parse of the .vvp file to fill in the details of the
	// property for the class definition.
      void set_property(size_t idx, const std::string&name, const std::string&type, uint64_t array_size);

	// This method is called after all the properties are
	// defined. This calculates information about the definition.
      void finish_setup(void);

    public:
	// Constructors and destructors for making instances.
      inst_t instance_new() const;
      void instance_delete(inst_t) const;

      void set_vec4(inst_t inst, size_t pid, const vvp_vector4_t&val) const;
      void get_vec4(inst_t inst, size_t pid, vvp_vector4_t&val) const;
      void set_real(inst_t inst, size_t pid, double val) const;
      double get_real(inst_t inst, size_t pid) const;
      void set_string(inst_t inst, size_t pid, const std::string&val) const;
      std::string get_string(inst_t inst, size_t pid) const;
      void set_object(inst_t inst, size_t pid, const vvp_object_t&val, size_t idx) const;
      void get_object(inst_t inst, size_t pid, vvp_object_t&val, size_t idx) const;

      void copy_property(inst_t dst, size_t idx, inst_t src) const;

    public: // VPI related methods
      int get_type_code(void) const;

    private:
      std::string class_name_;

      struct prop_t {
	    std::string name;
	    class_property_t*type;
      };
      std::vector<prop_t> properties_;
      size_t instance_size_;
};

#endif /* IVL_class_type_H */
