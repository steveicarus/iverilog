/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
 * Copyright CERN 2016
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

# define __STDC_LIMIT_MACROS
# include  "parse_misc.h"
# include  "compiler.h"
# include  "package.h"
# include  "std_types.h"
# include  "std_funcs.h"
# include  <fstream>
# include  <list>
# include  <map>
# include  <string>
# include  <stdint.h>
# include  <sys/stat.h>
# include  <cassert>

using namespace std;

/*
 * The library_work_path is the path to the work directory.
 */
static const char*library_work_path = 0;

/*
 * The library_search_path is a list of directory names to search to
 * find a named library. The library name is the name given to the
 * "import" statement.
 */
static list<string> library_search_path;

/*
 * The "import" statement causes me to build a map of a library name
 * to a library directory.
 */
static map<perm_string,string> library_dir;

/*
 * The "libraries" table maps library name to a set of packages. This
 * map is filled in by "use" statements.
 */
struct library_contents {
      map<perm_string,Package*> packages;
};
static map<perm_string,struct library_contents> libraries;

void library_add_directory(const char*directory)
{
	// Make sure the directory path really is a directory. Ignore
	// it if it is not.
      struct stat stat_buf;
      int rc = stat(directory, &stat_buf);
      if (rc < 0 || !S_ISDIR(stat_buf.st_mode)) {
	    return;
      }

      library_search_path.push_front(directory);
}

SubprogramHeader*library_match_subprogram(perm_string name, const list<const VType*>*params)
{
      SubprogramHeader*subp;
      map<perm_string,struct library_contents>::const_iterator lib_it;

      for(lib_it = libraries.begin(); lib_it != libraries.end(); ++lib_it) {
        const struct library_contents&lib = lib_it->second;
        map<perm_string,Package*>::const_iterator pack_it;

        for(pack_it = lib.packages.begin(); pack_it != lib.packages.end(); ++pack_it) {
            if((subp = pack_it->second->match_subprogram(name, params)))
                return subp;
        }
      }

      return NULL;
}

static void store_package_in_work(const Package*pack);
static string make_work_package_path(const char*name)
{
      return string(library_work_path).append("/").append(name).append(".pkg");
}

static string make_library_package_path(perm_string lib_name, perm_string name)
{
      string path = library_dir[lib_name];
      if (path == "")
	    return "";

      path = path.append("/").append(name).append(".pkg");
      return path;
}

static void import_ieee(void);
static void import_ieee_use(const YYLTYPE&loc, ActiveScope*res, perm_string package, perm_string name);
static void import_std_use(const YYLTYPE&loc, ActiveScope*res, perm_string package, perm_string name);

static void dump_library_package(ostream&file, perm_string lname, perm_string pname, Package*pack)
{
      file << "package " << lname << "." << pname << endl;
      if (pack) {
	    pack->dump_scope(file);
      } else {
	    file << "   <missing>" << endl;
      }
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
 * library if necessary. The parser uses this when it is finished with
 * a package declaration.
 */
void library_save_package(perm_string parse_library_name, Package*pack)
{
      perm_string use_libname = parse_library_name.str()
	    ? parse_library_name
	    : perm_string::literal("work");
      struct library_contents&lib = libraries[use_libname];

      lib.packages[pack->name()] = pack;

	// If this is a work package, and we are NOT parsing the work
	// library right now, then store it in the work library.
      if (parse_library_name.str() == 0)
	    store_package_in_work(pack);
      else
	    pack->set_library(parse_library_name);
}

/*
 * The parser uses this function in the package body rule to recall
 * the package that was declared earlier.
 */
Package*library_recall_package(perm_string parse_library_name, perm_string package_name)
{
      perm_string use_libname = parse_library_name.str()
	    ? parse_library_name
	    : perm_string::literal("work");

      map<perm_string,struct library_contents>::iterator lib = libraries.find(use_libname);
      if (lib == libraries.end())
	    return 0;

      map<perm_string,Package*>::iterator pkg = lib->second.packages.find(package_name);
      if (pkg == lib->second.packages.end())
	    return 0;

      return pkg->second;
}

static void import_library_name(const YYLTYPE&loc, perm_string name)
{
      if (library_dir[name] != string())
	    return;

      for (list<string>::const_iterator cur = library_search_path.begin()
		 ; cur != library_search_path.end() ; ++cur) {
	    string curdir = *cur;
	    string try_path = curdir.append("/").append(name);

	    struct stat stat_buf;
	    int rc = stat(try_path.c_str(), &stat_buf);
	    if (rc < 0)
		  continue;
	    if (!S_ISDIR(stat_buf.st_mode))
		  continue;

	    library_dir[name] = try_path;
	    return;
      }

      errormsg(loc, "library import cannot find library %s\n", name.str());
}

void library_import(const YYLTYPE&loc, const std::list<perm_string>*names)
{
      for (std::list<perm_string>::const_iterator cur = names->begin()
		 ; cur != names->end() ; ++cur) {
	    if (*cur == "ieee") {
		    // The ieee library is special and handled by an
		    // internal function.
		  import_ieee();
	    } else if (*cur == "std") {
		    // The std library is always implicitly imported.

	    } else if (*cur == "work") {
		    // The work library is always implicitly imported.

	    } else {
		    // Otherwise, do a generic library import
		  import_library_name(loc, *cur);
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
	    import_ieee_use(loc, res, use_package, use_name);
	    return;
      }
	// Special case handling for the STD library.
      if (use_library == "std") {
	    import_std_use(loc, res, use_package, use_name);
	    return;
      }

      struct library_contents&lib = libraries[use_library];
      Package*pack = lib.packages[use_package];
	// If the package is not found in the work library already
	// parsed, then see if it exists unparsed.
      if (use_library=="work" && pack == 0) {
	    string path = make_work_package_path(use_package.str());
	    parse_source_file(path.c_str(), use_library);
	    pack = lib.packages[use_package];
      } else if (use_library != "ieee" && pack == 0) {
	    string path = make_library_package_path(use_library, use_package);
	    if (path == "") {
		  errormsg(loc, "Unable to find library %s\n", use_library.str());
		  return;
	    }
	    int rc = parse_source_file(path.c_str(), use_library);
	    if (rc < 0)
		  errormsg(loc, "Unable to open library file %s\n", path.c_str());
	    else if (rc > 0)
		  errormsg(loc, "Errors in library file %s\n", path.c_str());
	    else
		  pack = lib.packages[use_package];
      }

	// If the package is still not found, then error.
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
	    res->use_name(perm_string::literal("std_logic_vector"), &primitive_STDLOGIC_VECTOR);
      }
}

static void import_ieee_use_std_logic_misc(ActiveScope*, perm_string name)
{
      bool all_flag = name=="all";
      list<InterfacePort*>*args;

      if (all_flag || name == "or_reduce") {
            /* function or_reduce(arg : std_logic_vector) return std_logic;
            */
            args = new list<InterfacePort*>();
            args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
            register_std_subprogram(new SubprogramBuiltin(perm_string::literal("or_reduce"),
                                                perm_string::literal("|"),
                                                args, &primitive_STDLOGIC));
      }

      if (all_flag || name == "and_reduce") {
            /* function and_reduce(arg : std_logic_vector) return std_logic;
            */
            args = new list<InterfacePort*>();
            args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
            register_std_subprogram(new SubprogramBuiltin(perm_string::literal("and_reduce"),
                                                perm_string::literal("&"),
                                                args, &primitive_STDLOGIC));
      }
}

static void import_ieee_use_numeric_bit(ActiveScope*res, perm_string name)
{
      bool all_flag = name=="all";

      if (all_flag || name == "signed") {
	    res->use_name(perm_string::literal("signed"), &primitive_SIGNED);
      }
      if (all_flag || name == "unsigned") {
	    res->use_name(perm_string::literal("unsigned"), &primitive_UNSIGNED);
      }
}

static void import_ieee_use_numeric_std(ActiveScope*res, perm_string name)
{
      bool all_flag = name=="all";

      if (all_flag || name == "signed") {
	    res->use_name(perm_string::literal("signed"), &primitive_SIGNED);
      }
      if (all_flag || name == "unsigned") {
	    res->use_name(perm_string::literal("unsigned"), &primitive_UNSIGNED);
      }
}

static void import_ieee_use(const YYLTYPE&/*loc*/, ActiveScope*res,
        perm_string package, perm_string name)
{
      if (package == "std_logic_1164") {
	    import_ieee_use_std_logic_1164(res, name);
	    return;
      }

      if (package == "std_logic_arith") {
            // arithmetic operators for std_logic_vector
	    return;
      }

      if (package == "std_logic_misc") {
            import_ieee_use_std_logic_misc(res, name);
            return;
      }

      if (package == "std_logic_unsigned") {
            // arithmetic operators for std_logic_vector
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

      cerr << "Warning: Package ieee." << package.str() <<" is not yet supported" << endl;
}

static void import_std_use(const YYLTYPE&/*loc*/, ActiveScope*res,
        perm_string package, perm_string /*name*/)
{
      if (package == "standard") {
	    // do nothing
	    return;
      } else if (package == "textio") {
            res->use_name(perm_string::literal("text"),     &primitive_INTEGER);
            res->use_name(perm_string::literal("line"),     &primitive_STRING);
            res->use_name(type_FILE_OPEN_KIND.peek_name(),  &type_FILE_OPEN_KIND);
            res->use_name(type_FILE_OPEN_STATUS.peek_name(),  &type_FILE_OPEN_STATUS);
	    return;
      } else {
            cerr << "Warning: Package std." << package.str() <<" is not yet supported" << endl;
	    return;
      }
}

void library_set_work_path(const char*path)
{
      assert(library_work_path == 0);
      library_work_path = path;
}

list<const Package*> work_packages;
static void store_package_in_work(const Package*pack)
{
      work_packages.push_back(pack);
}

static int emit_packages(perm_string, const map<perm_string,Package*>&packages)
{
      int errors = 0;
      for (map<perm_string,Package*>::const_iterator cur = packages.begin()
		 ; cur != packages.end() ; ++cur) {
	    errors += cur->second->emit_package(cout);
      }

      for (list<const Package*>::const_iterator cur = work_packages.begin()
		 ; cur != work_packages.end(); ++cur) {
	    string path = make_work_package_path((*cur)->name());
	    ofstream file (path.c_str(), ios_base::out);
	    (*cur)->write_to_stream(file);
      }

      return errors;
}

int emit_packages(void)
{
      int errors = 0;
      for (map<perm_string,struct library_contents>::iterator cur = libraries.begin()
		 ; cur != libraries.end() ; ++cur) {
	    errors += emit_packages(cur->first, cur->second.packages);
      }

      return errors;
}

static int elaborate_library_packages(map<perm_string,Package*>packages)
{
      int errors = 0;

      for (map<perm_string,Package*>::iterator cur = packages.begin()
		 ; cur != packages.end() ;  ++cur) {
	    errors += cur->second->elaborate();
      }

      return errors;
}

int elaborate_libraries()
{
      int errors = 0;

      for (map<perm_string,struct library_contents>::iterator cur = libraries.begin()
		 ; cur != libraries.end() ;  ++cur) {
	    errors += elaborate_library_packages(cur->second.packages);
      }

      return errors;
}
