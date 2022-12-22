/*
 * Copyright (c) 2012-2021 Stephen Williams (steve@icarus.com)
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
	    VLerror(loc, msg.str().c_str());
      }


      packages_by_name[use_name] = pform_cur_package;
      pform_packages.push_back(pform_cur_package);
      pform_cur_package = 0;
      pform_pop_scope();
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
	    map<perm_string,PNamedItem*>::const_iterator cur_sym
		  = pkg->local_symbols.find(use_ident);
	    if (cur_sym == pkg->local_symbols.end()) {
		  cerr << loc.get_fileline() << ": error: "
			  "'" << use_ident << "' is not exported by '"
		       << pkg->pscope_name() << "'." << endl;
		  error_count += 1;
		  return;
	    }

	      // Check for conflict with local symbol.
	    cur_sym = scope->local_symbols.find(use_ident);
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
		  if (cur_pkg->second != pkg) {
			cerr << loc.get_fileline() << ": error: "
				"'" << use_ident << "' has already been "
				"imported into this scope from package '"
			     << cur_pkg->second->pscope_name() << "'." << endl;
			error_count += 1;
		  }
		  return;
	    }

	    scope->explicit_imports[use_ident] = pkg;

      } else {
	    list<PPackage*>::const_iterator cur_pkg
		  = find(scope->potential_imports.begin(),
                         scope->potential_imports.end(),
                         pkg);
	    if (cur_pkg == scope->potential_imports.end())
		  scope->potential_imports.push_back(pkg);
      }
}

PExpr* pform_package_ident(const struct vlltype&loc,
			   PPackage*pkg, pform_name_t*ident_name)
{
      assert(ident_name);
      PEIdent*tmp = new PEIdent(pkg, *ident_name);
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
