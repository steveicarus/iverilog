/*
 * Copyright (c) 2012-2024 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 Stephen Williams (steve@icarus.com)
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

# include  <cstdarg>
# include  "pform.h"
# include  "PPackage.h"
# include  "parse_misc.h"
# include  "parse_api.h"
# include  <map>
# include  <sstream>
# include  "ivl_assert.h"

using namespace std;

/*
 * This is a list of packages in the order that they were defined.
 */
vector<PPackage*> pform_packages;

/*
 * This allows us to easily check for name collisions.
 */
static map<perm_string,PPackage*> packages_by_name;

static PPackage*pform_cur_package = 0;

void pform_start_package_declaration(const struct vlltype&loc, const char*name,
				     LexicalScope::lifetime_t lifetime)
{
      ivl_assert(loc, pform_cur_package == 0);

      perm_string use_name = lex_strings.make(name);
      PPackage*pkg_scope = pform_push_package_scope(loc, use_name, lifetime);
      FILE_NAME(pkg_scope, loc);
      pform_cur_package = pkg_scope;
}

void pform_end_package_declaration(const struct vlltype&loc)
{
      ivl_assert(loc, pform_cur_package);
      perm_string use_name = pform_cur_package->pscope_name();

      map<perm_string,PPackage*>::const_iterator test = packages_by_name.find(use_name);
      if (test != packages_by_name.end()) {
	    ostringstream msg;
	    msg << "error: Package " << use_name << " was already declared here: "
		<< test->second->get_fileline() << ends;
	    VLerror(loc, "%s", msg.str().c_str());
      }


      packages_by_name[use_name] = pform_cur_package;
      pform_packages.push_back(pform_cur_package);
      pform_cur_package = 0;
      pform_pop_scope();
}

PPackage *pform_find_potential_import(const struct vlltype&loc, LexicalScope*scope,
				      perm_string name, bool tf_call, bool make_explicit)
{
      ivl_assert(loc, scope);

      PPackage *found_pkg = nullptr;
      for (auto search_pkg : scope->potential_imports) {
	    PPackage *decl_pkg = pform_package_importable(search_pkg, name);
	    if (!decl_pkg)
		  continue;

	    if (found_pkg && found_pkg != decl_pkg && make_explicit) {
		  cerr << loc.get_fileline() << ": error: "
			  "Ambiguous use of '" << name << "'. "
			  "It is exported by both '"
			<< found_pkg->pscope_name()
			<< "' and by '"
			<< search_pkg->pscope_name()
			<< "'." << endl;
		  error_count++;
		  continue;
	    }

	    found_pkg = decl_pkg;
	    if (make_explicit) {
		  if (tf_call)
			scope->possible_imports[name] = found_pkg;
		  else {
			scope->explicit_imports[name] = found_pkg;
			scope->explicit_imports_from[name].insert(search_pkg);
		  }
	    }
      }

      return found_pkg;
}

PPackage *pform_package_importable(PPackage *pkg, perm_string name)
{
	if (pkg->local_symbols.find(name) != pkg->local_symbols.end())
		  return pkg;

	auto import_pkg = pkg->explicit_imports.find(name);
	if (import_pkg == pkg->explicit_imports.end())
		return nullptr;

	for (auto &exp : pkg->exports) {
		  // *::* will match all imports, P::* will match all imports
		  // from a package and P::ID will match a specific identifier
		  // from a package.
		if ((!exp.pkg || exp.pkg == import_pkg->second) &&
		    (exp.name.nil() || exp.name == name))
			return import_pkg->second;
	}

	return nullptr;
}

/*
 * Do the import early, during processing. This requires that the
 * package is declared in pform ahead of time (it is) and that we can
 * simply transfer definitions to the current scope (we can).
 */
void pform_package_import(const struct vlltype&loc, PPackage*pkg, const char*ident)
{
      LexicalScope*scope = pform_peek_scope();

      if (ident) {
	    perm_string use_ident = lex_strings.make(ident);

	      // Check that the requested symbol is available.
	    PPackage *pkg_decl = pform_package_importable(pkg, use_ident);
	    if (!pkg_decl) {
		  cerr << loc.get_fileline() << ": error: "
			  "'" << use_ident << "' is not exported by '"
		       << pkg->pscope_name() << "'." << endl;
		  error_count += 1;
		  return;
	    }

	      // Check for conflict with local symbol.
	    auto cur_sym = scope->local_symbols.find(use_ident);
	    if (cur_sym != scope->local_symbols.end()) {
		  cerr << loc.get_fileline() << ": error: "
			  "'" << use_ident << "' has already been declared "
			  "in this scope." << endl;
		  cerr << cur_sym->second->get_fileline() << ":      : "
			  "It was declared here as "
		       << cur_sym->second->symbol_type() << "." << endl;
		  error_count += 1;
		  return;
	    }

	      // Check for conflict with previous import.
	    map<perm_string,PPackage*>::const_iterator cur_pkg
		  = scope->explicit_imports.find(use_ident);
	    if (cur_pkg != scope->explicit_imports.end()) {
		  if (cur_pkg->second != pkg_decl) {
			cerr << loc.get_fileline() << ": error: "
				"'" << use_ident << "' has already been "
				"imported into this scope from package '"
			     << cur_pkg->second->pscope_name() << "'." << endl;
			error_count += 1;
		  }
	    }

	    scope->explicit_imports[use_ident] = pkg_decl;
	    scope->explicit_imports_from[use_ident].insert(pkg);

      } else {
	    list<PPackage*>::const_iterator cur_pkg
		  = find(scope->potential_imports.begin(),
                         scope->potential_imports.end(),
                         pkg);
	    if (cur_pkg == scope->potential_imports.end())
		  scope->potential_imports.push_back(pkg);
      }
}

static bool pform_package_exportable(const struct vlltype &loc, PPackage *pkg,
				     const perm_string &ident)
{
      auto import_pkg = pform_cur_package->explicit_imports_from.find(ident);
      if (import_pkg != pform_cur_package->explicit_imports_from.end()) {
	    auto &pkg_list = import_pkg->second;
	    if (pkg_list.find(pkg) != pkg_list.end())
		  return true;
      }

      if (pform_cur_package->local_symbols.find(ident) == pform_cur_package->local_symbols.end()) {
	    if (pform_find_potential_import(loc, pform_cur_package,
					    ident, false, true))
		  return true;
      }

      cerr << loc.get_fileline() << ": error: "
	      "`" << ident << "` has not been imported from "
	   << pkg->pscope_name() << "." << endl;
      error_count++;

      return false;
}

void pform_package_export(const struct vlltype &loc, PPackage *pkg, const char *ident)
{
      ivl_assert(loc, pform_cur_package);

      perm_string use_ident;
      if (ident) {
	    use_ident = lex_strings.make(ident);
	    if (!pform_package_exportable(loc, pkg, use_ident))
		  return;
      }
      pform_cur_package->exports.push_back(PPackage::export_t{pkg, use_ident});
}

PExpr* pform_package_ident(const struct vlltype&loc,
			   PPackage*pkg, pform_name_t*ident_name)
{
      ivl_assert(loc, ident_name);
      PEIdent*tmp = new PEIdent(pkg, *ident_name, loc.lexical_pos);
      FILE_NAME(tmp, loc);
      return tmp;
}

typedef_t* pform_test_type_identifier(PPackage*pkg, const char*txt)
{
      perm_string use_name = lex_strings.make(txt);
      LexicalScope::typedef_map_t::const_iterator cur;

      cur = pkg->typedefs.find(use_name);
      if (cur != pkg->typedefs.end())
	    return cur->second;

      return 0;
}

/*
 * The lexor uses this function to know if the identifier names the
 * package. It will call this a PACKAGE_IDENTIFIER token in that case,
 * instead of a generic IDENTIFIER.
 */
PPackage* pform_test_package_identifier(const char*pkg_name)
{
      perm_string use_name = lex_strings.make(pkg_name);
      map<perm_string,PPackage*>::const_iterator pcur = packages_by_name.find(use_name);
      if (pcur == packages_by_name.end())
	    return 0;

      assert(pcur->second);
      return pcur->second;
}
