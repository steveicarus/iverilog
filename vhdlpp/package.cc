/*
 * Copyright (c) 2011-2013 Stephen Williams (steve@icarus.com)
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

# include  "package.h"
# include  "entity.h"
# include  "parse_misc.h"
# include  "ivl_assert.h"

Package::Package(perm_string n, const ActiveScope&ref)
: Scope(ref), name_(n)
{
}

Package::~Package()
{
    ScopeBase::cleanup();
}

void Package::set_library(perm_string lname)
{
      ivl_assert(*this, from_library_.str() == 0);
      from_library_ = lname;
}

/*
 * The Package::write_to_stream is used to write the package to the
 * work space (or library) so writes proper VHDL that the library
 * parser can bring back in as needed.
 */
void Package::write_to_stream(ostream&fd) const
{
      ivl_assert(*this, new_subprograms_.size() == 0);

      fd << "package " << name_ << " is" << endl;

	// Start out pre-declaring all the type definitions so that
	// there is no confusion later in the package between types
	// and identifiers.
      for (map<perm_string,const VType*>::const_iterator cur = use_types_.begin()
		 ; cur != use_types_.end() ; ++cur) {
	    const VTypeDef*def = dynamic_cast<const VTypeDef*> (cur->second);
	    if (def == 0)
		  continue;
	    fd << "type " << cur->first << ";" << endl;
      }
      for (map<perm_string,const VType*>::const_iterator cur = cur_types_.begin()
		 ; cur != cur_types_.end() ; ++cur) {
	    const VTypeDef*def = dynamic_cast<const VTypeDef*> (cur->second);
	    if (def == 0)
		  continue;
	    fd << "type " << cur->first << ";" << endl;
      }

      for (map<perm_string,struct const_t*>::const_iterator cur = cur_constants_.begin()
		 ; cur != cur_constants_.end() ; ++ cur) {
	    fd << "constant " << cur->first << ": ";
	    cur->second->typ->write_to_stream(fd);
	    fd << " := ";
	    cur->second->val->write_to_stream(fd);
	    fd << ";" << endl;
      }

      for (map<perm_string,const VType*>::const_iterator cur = use_types_.begin()
		 ; cur != use_types_.end() ; ++cur) {

	      // Do not include global types in types dump
	    if (is_global_type(cur->first))
		  continue;
	    if (cur->first == "std_logic_vector")
		  continue;

	    fd << "type " << cur->first << " is ";
	    cur->second->write_type_to_stream(fd);
	    fd << "; -- imported" << endl;
      }
      for (map<perm_string,const VType*>::const_iterator cur = cur_types_.begin()
		 ; cur != cur_types_.end() ; ++cur) {

	      // Do not include global types in types dump
	    if (is_global_type(cur->first))
		  continue;
	    if (cur->first == "std_logic_vector")
		  continue;

	    fd << "type " << cur->first << " is ";
	    cur->second->write_type_to_stream(fd);
	    fd << ";" << endl;
      }

      for (map<perm_string,Subprogram*>::const_iterator cur = old_subprograms_.begin()
		 ; cur != old_subprograms_.end() ; ++cur) {
	    cur->second->write_to_stream(fd);
      }

      for (map<perm_string,ComponentBase*>::const_iterator cur = old_components_.begin()
		 ; cur != old_components_.end() ; ++cur) {

	    cur->second->write_to_stream(fd);
      }
      for (map<perm_string,ComponentBase*>::const_iterator cur = new_components_.begin()
		 ; cur != new_components_.end() ; ++cur) {

	    cur->second->write_to_stream(fd);
      }

      fd << "end package " << name_ << ";" << endl;
}
