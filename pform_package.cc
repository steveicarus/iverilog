/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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

# include  "pform.h"
# include  "PPackage.h"
# include  "parse_misc.h"
# include  "parse_api.h"
# include  <map>
# include  <sstream>
# include  "ivl_assert.h"

using namespace std;

/*
 * This is a map of packages that have been defined.
 */
map<perm_string,PPackage*> pform_packages;

static PPackage*pform_cur_package = 0;

void pform_start_package_declaration(const struct vlltype&loc, const char*name)
{
      ivl_assert(loc, pform_cur_package == 0);

      perm_string use_name = lex_strings.make(name);
      PPackage*pkg_scope = pform_push_package_scope(loc, use_name);
      FILE_NAME(pkg_scope, loc);
      pform_cur_package = pkg_scope;
}

void pform_end_package_declaration(const struct vlltype&loc)
{
      ivl_assert(loc, pform_cur_package);
      perm_string use_name = pform_cur_package->pscope_name();

      map<perm_string,PPackage*>::const_iterator test = pform_packages.find(use_name);
      if (test != pform_packages.end()) {
	    ostringstream msg;
	    msg << "Package " << use_name << " was already declared here: "
		<< test->second->get_fileline() << ends;
	    VLerror(msg.str().c_str());
      } else {
	    pform_packages[use_name] = pform_cur_package;
      }


      pform_packages[use_name] = pform_cur_package;
      pform_cur_package = 0;
      pform_pop_scope();
}

/*
 * Do the import early, during processing. This requires that the
 * package is declared in pform ahead of time (it is) and that we can
 * simply transfer definitions to the current scope (we can).
 */
void pform_package_import(const struct vlltype&, PPackage*pkg, const char*ident)
{
      LexicalScope*scope = pform_peek_scope();

      if (ident) {
	    perm_string use_ident = lex_strings.make(ident);

	    map<perm_string,LexicalScope::param_expr_t>::const_iterator cur
		  = pkg->parameters.find(use_ident);
	    if (cur != pkg->parameters.end()) {
		  scope->imports[cur->first] = pkg;
		  return;
	    }

	    cur = pkg->localparams.find(use_ident);
	    if (cur != pkg->localparams.end()) {
		  scope->imports[cur->first] = pkg;
		  return;
	    }

	    map<perm_string,data_type_t*>::const_iterator tcur;
	    tcur = pkg->typedefs.find(use_ident);
	    if (tcur != pkg->typedefs.end()) {
		  scope->imports[tcur->first] = pkg;
		  return;
	    }

	    map<perm_string,PFunction*>::const_iterator fcur;
	    fcur = pkg->funcs.find(use_ident);
	    if (fcur != pkg->funcs.end()) {
		  scope->imports[fcur->first] = pkg;
		  return;
	    }

	    map<perm_string,PTask*>::const_iterator ttcur;
	    ttcur = pkg->tasks.find(use_ident);
	    if (ttcur != pkg->tasks.end()) {
		  scope->imports[ttcur->first] = pkg;
		  return;
	    }

	    map<perm_string,PWire*>::const_iterator wcur;
	    wcur = pkg->wires.find(use_ident);
	    if (wcur != pkg->wires.end()) {
		  scope->imports[wcur->first] = pkg;
		  return;
	    }

	    ostringstream msg;
	    msg << "Symbol " << use_ident
		<< " not found in package " << pkg->pscope_name() << "." << ends;
	    VLerror(msg.str().c_str());
	    return;

      } else {

	      // Handle the pkg::* case by importing everything from
	      // the package.
	    for (map<perm_string,LexicalScope::param_expr_t>::const_iterator cur = pkg->parameters.begin()
		       ; cur != pkg->parameters.end() ; ++cur) {

		  scope->imports[cur->first] = pkg;
	    }

	    for (map<perm_string,LexicalScope::param_expr_t>::const_iterator cur = pkg->localparams.begin()
		       ; cur != pkg->localparams.end() ; ++cur) {

		  scope->imports[cur->first] = pkg;
	    }

	    for (map<perm_string,data_type_t*>::const_iterator cur = pkg->typedefs.begin()
		       ; cur != pkg->typedefs.end() ; ++cur) {

		  scope->imports[cur->first] = pkg;
	    }

	    for (map<perm_string,PFunction*>::const_iterator cur = pkg->funcs.begin()
		       ; cur != pkg->funcs.end() ; ++cur) {

		  scope->imports[cur->first] = pkg;
	    }
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

data_type_t* pform_test_type_identifier(PPackage*pkg, const char*txt)
{
      perm_string use_name = lex_strings.make(txt);
      map<perm_string,data_type_t*>::const_iterator cur = pkg->typedefs.find(use_name);
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
      map<perm_string,PPackage*>::const_iterator pcur = pform_packages.find(use_name);
      if (pcur == pform_packages.end())
	    return 0;

      assert(pcur->second);
      return pcur->second;
}
