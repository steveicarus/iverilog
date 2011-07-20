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
# include  <fstream>
# include  <map>
# include  <string>
# include  <cassert>

using namespace std;

static const char*library_work_path = 0;
static void store_package_in_work(const Package*pack);

struct library_contents {
      map<perm_string,Package*> packages;
};

static void import_ieee(void);
static void import_ieee_use(ActiveScope*res, perm_string package, perm_string name);

static map<perm_string,struct library_contents> libraries;

static void dump_library_package(ostream&file, perm_string lname, perm_string pname, Package*pack)
{
      file << "package " << lname << "." << pname << endl;
      pack->dump_scope(file);
      file << "end package " << lname << "." << pname << endl;
}

static void dump_library_packages(ostream&file, perm_string lname, map<perm_string,Package*>packages)
{
      for (map<perm_string,Package*>::iterator cur = packages.begin()
		 ; cur != packages.end() ;  ++cur) {
	    dump_library_package(file, lname, cur->first, cur->second);
      }
}

void dump_libraries(ostream&file)
{
      for (map<perm_string,struct library_contents>::iterator cur = libraries.begin()
		 ; cur != libraries.end() ;  ++cur) {
	    dump_library_packages(file, cur->first, cur->second.packages);
      }
}

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

      if (use_libname == "work")
	    store_package_in_work(pack);
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

void library_use(const YYLTYPE&loc, ActiveScope*res,
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
	    import_ieee_use(res, use_package, use_name);
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
	    res->use_from(pack);
	    return;
      }

      if (ComponentBase*cur = pack->find_component(use_name)) {
	    res->bind_name(use_name, cur);
	    return;
      }

      errormsg(loc, "No such name %s in package %s\n",
	       use_name.str(), pack->name().str());
}

static void import_ieee(void)
{
}

static void import_ieee_use_std_logic_1164(ActiveScope*res, perm_string name)
{
      bool all_flag = name=="all";

      if (all_flag || name == "std_logic_vector") {
	    vector<VTypeArray::range_t> dims (1);
	    res->bind_name(perm_string::literal("std_logic_vector"),
			   new VTypeArray(&primitive_STDLOGIC, dims, false));
      }
}

static void import_ieee_use_numeric_bit(ActiveScope*res, perm_string name)
{
      bool all_flag = name=="all";

      if (all_flag || name == "signed") {
	    vector<VTypeArray::range_t> dims (1);
	    res->bind_name(perm_string::literal("signed"),
			   new VTypeArray(&primitive_STDLOGIC, dims, true));
      }
      if (all_flag || name == "unsigned") {
	    vector<VTypeArray::range_t> dims (1);
	    res->bind_name(perm_string::literal("unsigned"),
			   new VTypeArray(&primitive_BIT, dims, false));
      }
}

static void import_ieee_use_numeric_std(ActiveScope*res, perm_string name)
{
      bool all_flag = name=="all";

      if (all_flag || name == "signed") {
	    vector<VTypeArray::range_t> dims (1);
	    res->bind_name(perm_string::literal("signed"),
			   new VTypeArray(&primitive_STDLOGIC, dims, true));
      }
      if (all_flag || name == "unsigned") {
	    vector<VTypeArray::range_t> dims (1);
	    res->bind_name(perm_string::literal("unsigned"),
			   new VTypeArray(&primitive_STDLOGIC, dims, false));
      }
}

static void import_ieee_use(ActiveScope*res, perm_string package, perm_string name)
{
      if (package == "std_logic_1164") {
	    import_ieee_use_std_logic_1164(res, name);
	    return;
      }

      if (package == "numeric_bit") {
	    import_ieee_use_numeric_bit(res, name);
	    return;
      }

      if (package == "numeric_std") {
	    import_ieee_use_numeric_std(res, name);
	    return;
      }
}


const VTypePrimitive primitive_BOOLEAN (VTypePrimitive::BOOLEAN);
const VTypePrimitive primitive_BIT     (VTypePrimitive::BIT);
const VTypePrimitive primitive_INTEGER (VTypePrimitive::INTEGER);
const VTypePrimitive primitive_STDLOGIC(VTypePrimitive::STDLOGIC);

const VTypeArray primitive_BIT_VECTOR(&primitive_BIT,      vector<VTypeArray::range_t> (1));
const VTypeArray primitive_BOOL_VECTOR(&primitive_BOOLEAN, vector<VTypeArray::range_t> (1));

void generate_global_types(ActiveScope*res)
{
      res->bind_name(perm_string::literal("boolean"),   &primitive_BOOLEAN);
      res->bind_name(perm_string::literal("bit"),       &primitive_BIT);
      res->bind_name(perm_string::literal("integer"),   &primitive_INTEGER);
      res->bind_name(perm_string::literal("std_logic"), &primitive_STDLOGIC);
      res->bind_name(perm_string::literal("bit_vector"),&primitive_BOOL_VECTOR);
}

void library_set_work_path(const char*path)
{
      assert(library_work_path == 0);
      library_work_path = path;
}

static void store_package_in_work(const Package*pack)
{
      string path = string(library_work_path).append("/").append(pack->name()).append(".pkg");

      ofstream file (path.c_str(), ios_base::out|ios_base::app);

      pack->write_to_stream(file);
}
