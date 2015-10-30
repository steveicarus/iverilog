#ifndef IVL_nex_data_H
#define IVL_nex_data_H
/*
 * Copyright (c) 2013-2014 Stephen Williams (steve@icarus.com)
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
# include  <vector>
# include  <cstddef>

/*
 * The ivl_target.h API allows for binding data to a nexus. This class
 * represents the data that we want to attach to a nexus.
 */
class blif_nex_data_t {

    private:
	// The constructors are private. Only the get_nex_data()
	// function can create these objects.
      explicit blif_nex_data_t(ivl_nexus_t nex);
      ~blif_nex_data_t();

    public:
	// Return the blif_nex_data_t object that is associated with
	// the given nexus. If the nexus does not have a nex_data_t
	// object, then create it and bind it to the nexus. Thus, this
	// function will always return the same nex_data instance for
	// the same nexus.
      static blif_nex_data_t* get_nex_data(ivl_nexus_t nex);

	// In certain situations, we know a priori what we want the
	// nexus name to be. In those cases, the context can use this
	// method to set the name (by the signal from width the name
	// is derived). Note that this must be called before the name
	// is otherwise queried.
      void set_name(ivl_signal_t sig);

	// Get the symbolic name chosen for this nexus.
      const char*get_name(void);

	// Map a canonical bit index (0 : width-1) to the bit number
	// as understood by the signal. This is normally a null
	// mapping, but sometimes the signal name used for the mapping
	// has a non-canonical bit numbering.
      const char*get_name_index(unsigned bit);

	// Get the vector width for this nexus.
      size_t get_width(void);

    public:
      ivl_nexus_t nex_;
      char*name_;
      std::vector<char*> name_index_;

    private:
      void select_name_(void);
      void make_name_from_sig_(ivl_signal_t sig);
};

#endif /* IVL_nex_data_H */
