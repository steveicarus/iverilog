#ifndef __class_type_H
#define __class_type_H
/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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

/*
 * This represents the TYPE information for a class. A %new operator
 * uses this information to figure out how to construct an actual
 * instance.
 */
class class_type : public __vpiHandle {

    public:
      explicit class_type(const std::string&nam, size_t nprop);

	// This is the name of the class type.
      inline const std::string&class_name(void) const { return class_name_; }
	// Number of properties in the class definition.
      inline size_t property_count(void) const { return properties_.size(); }

	// Set the details about the property. This is used during
	// parse of the .vvp file to fill in the details of the
	// property for the class definition.
      void set_property(size_t idx, const std::string&name);

      int get_type_code(void) const;

    private:
      std::string class_name_;

      struct prop_t {
	    std::string name;
      };
      std::vector<prop_t> properties_;
};

#endif
