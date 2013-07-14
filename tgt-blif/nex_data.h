#ifndef __nex_data_H
#define __nex_data_H
/*
 * Copyright (c) 2013 Stephen Williams (steve@icarus.com)
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

# include  "ivl_target.h"

/*
 * The ivl_target.h API allows for binding data to a nexus. This class
 * represents the data that we want to attach to a nexus.
 */
class blif_nex_data_t {

    private:
	// The constructors are private. Only the get_nex_data()
	// function can create these objects.
      blif_nex_data_t(ivl_nexus_t nex);
      ~blif_nex_data_t();

    public:
	// Return the blif_nex_data_t object that is associated with
	// the given nexus. If the nexus does not have a nex_data_t
	// object, then create it and bind it to the nexus. Thus, this
	// function will always return the same nex_data instance for
	// the same nexus.
      static blif_nex_data_t* get_nex_data(ivl_nexus_t nex);

      void set_name(const char*);

	// Get the symbolic name chosen for this nexus.
      const char*get_name(void);

    public:
      ivl_nexus_t nex_;
      char*name_;
};

#endif
