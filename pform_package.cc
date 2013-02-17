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
void pform_package_import(const struct vlltype&, const char*pkg_name, const char*ident)
{
      perm_string use_name = lex_strings.make(pkg_name);
      map<perm_string,PPackage*>::const_iterator pcur = pform_packages.find(use_name);
      if (pcur == pform_packages.end()) {
	    ostringstream msg;
	    msg << "Package " << pkg_name << " not found." << ends;
	    VLerror(msg.str().c_str());
	    return;
      }

      PPackage*pkg = pcur->second;
      LexicalScope*scope = pform_peek_scope();

      if (ident) {
	    perm_string use_ident = lex_strings.make(ident);

	    map<perm_string,LexicalScope::param_expr_t>::const_iterator cur
		  = pkg->parameters.find(use_ident);
	    if (cur == pkg->parameters.end()) {
		  ostringstream msg;
		  msg << "Symbol " << use_ident
		      << " not found in package " << pcur->first << "." << ends;
		  VLerror(msg.str().c_str());
		  return;
	    }

	    scope->parameters[cur->first] = cur->second;

      } else {

	      // Handle the pkg::* case by importing everything from
	      // the package.
	    for (map<perm_string,LexicalScope::param_expr_t>::const_iterator cur = pkg->parameters.begin()
		       ; cur != pkg->parameters.end() ; ++cur) {

		  scope->parameters[cur->first] = cur->second;
	    }
      }
}

PExpr* pform_package_ident(const struct vlltype&loc,
			   const char*pkg_name, const char*ident_name)
{
      perm_string use_name = lex_strings.make(pkg_name);
      map<perm_string,PPackage*>::const_iterator pcur = pform_packages.find(use_name);
      if (pcur == pform_packages.end()) {
	    ostringstream msg;
	    msg << "Package " << pkg_name << " not found." << ends;
	    VLerror(msg.str().c_str());
	    return 0;
      }

      assert(pcur->second);
      perm_string use_ident = lex_strings.make(ident_name);
      PEIdent*tmp = new PEIdent(pcur->second, use_ident);
      FILE_NAME(tmp, loc);
      return tmp;
}
