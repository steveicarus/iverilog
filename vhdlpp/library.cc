/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "parse_misc.h"
# include  "compiler.h"
# include  "package.h"
# include  <map>

using namespace std;


struct library_contents {
      map<perm_string,Package*> packages;
};

static map<perm_string,struct library_contents> libraries;

/*
 * This function saves a package into the named library. Create the
 * library if necessary.
 */
void library_save_package(const char*libname, Package*pack)
{
      if (libname == 0)
	    libname = "work";

      perm_string use_libname = lex_strings.make(libname);
      struct library_contents&lib = libraries[use_libname];

      lib.packages[pack->name()] = pack;
}

void library_import(const YYLTYPE&loc, const std::list<perm_string>*names)
{
      for (std::list<perm_string>::const_iterator cur = names->begin()
		 ; cur != names->end() ; ++cur) {
	    if (*cur == "ieee") {
		    // The ieee library is special and handled by an
		    // internal function.
		  import_ieee();
	    } else {
		  sorrymsg(loc, "library import (%s) not implemented.\n", cur->str());
	    }
      }
}

void library_use(const YYLTYPE&loc, struct library_results&res,
		 const char*libname, const char*package, const char*name)
{
      if (libname == 0) {
	    errormsg(loc, "error: No library name for this use clause?\n");
	    return;
      }

      perm_string use_library = lex_strings.make(libname);
      perm_string use_package = lex_strings.make(package);
      perm_string use_name = name? lex_strings.make(name) : perm_string::literal("all");

	// Special case handling for the IEEE library.
      if (use_library == "ieee") {
	    import_ieee_use(use_package, use_name);
	    return;
      }

      struct library_contents&lib = libraries[use_library];
      Package*pack = lib.packages[use_package];
      if (pack == 0) {
	    errormsg(loc, "No package %s in library %s\n",
		     use_package.str(), use_library.str());
	    return;
      }

	// We have a package that we are going to extract names
	// from. Use the name to get the selected objects, and write
	// results into the "res" members.

      if (use_name == "all") {
	    pack->collect_components(res.components);
	    return;
      }

      if (ComponentBase*cur = pack->find_component(use_name)) {
	    res.components.push_back(cur);
	    return;
      }

      errormsg(loc, "No such name %s in package %s\n",
	       use_name.str(), pack->name().str());
}
