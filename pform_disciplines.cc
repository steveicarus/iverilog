/*
 * Copyright (c) 2008-2024 Stephen Williams (steve@icarus.com)
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
# include  "config.h"
# include  "compiler.h"
# include  "pform.h"
# include  "parse_misc.h"
# include  "discipline.h"
# include  "ivl_assert.h"

using namespace std;

map<perm_string,ivl_nature_t> natures;
map<perm_string,ivl_discipline_t> disciplines;
map<perm_string,ivl_nature_t> access_function_nature;

static perm_string nature_name = perm_string();
static perm_string nature_access = perm_string();

void pform_start_nature(const char*name)
{
      nature_name = lex_strings.make(name);
}

void pform_nature_access(const struct vlltype&loc, const char*name)
{
      if (nature_access) {
	    cerr << loc.text << ":" << loc.first_line << ": error: "
		 << "Too many access names for nature "
		 << nature_name << "." << endl;
	    error_count += 1;
	    return;
      }

      nature_access = lex_strings.make(name);
}

void pform_end_nature(const struct vlltype&loc)
{
	// The nature access function is required. If it is missing,
	// then signal an error. For a temporary expedient, we can set
	// the nature name as the access function, but don't expect it
	// to work.
      if (! nature_access) {
	    cerr << loc.text << ":" << loc.first_line << ": error: "
		 << "Missing access name for nature "
		 << nature_name << "." << endl;
	    error_count += 1;
	    nature_access = nature_name;
      }

      ivl_nature_s*tmp = new ivl_nature_s(nature_name, nature_access);
      FILE_NAME(tmp, loc);

      natures[nature_name] = tmp;

	// Make sure the access function is not used by multiple
	// different natures.
      if (ivl_nature_t dup_access_nat = access_function_nature[nature_access]) {
	    cerr << tmp->get_fileline() << ": error: "
		 << "Access function name " << nature_access
		 << " is already used by nature " << dup_access_nat->name()
		 << " declared at " << dup_access_nat->get_fileline()
		 << "." << endl;
	    error_count += 1;
      }

	// Map the access function back to the nature so that
	// expressions that use the access function can find it.
      access_function_nature[nature_access] = tmp;

      nature_name = perm_string();
      nature_access = perm_string();
}


static perm_string discipline_name;
static ivl_dis_domain_t discipline_domain = IVL_DIS_NONE;
static ivl_nature_t discipline_potential = 0;
static ivl_nature_t discipline_flow = 0;

void pform_start_discipline(const char*name)
{
      discipline_name = lex_strings.make(name);
      discipline_domain = IVL_DIS_NONE;
}

void pform_discipline_domain(const struct vlltype&loc, ivl_dis_domain_t use_domain)
{
      ivl_assert(loc, use_domain != IVL_DIS_NONE);

      if (discipline_domain != IVL_DIS_NONE) {
	    cerr << loc.text << ":" << loc.first_line << ": error: "
		 << "Too many domain attributes for discipline "
		 << discipline_name << "." << endl;
	    error_count += 1;
	    return;
      }

      discipline_domain = use_domain;
}

void pform_discipline_potential(const struct vlltype&loc, const char*name)
{
      if (discipline_potential) {
	    cerr << loc.text << ":" << loc.first_line << ": error: "
		 << "Too many potential natures for discipline "
		 << discipline_name << "." << endl;
	    error_count += 1;
	    return;
      }

      perm_string key = lex_strings.make(name);
      discipline_potential = natures[key];

      if (discipline_potential == 0) {
	    cerr << loc.text << ":" << loc.first_line << ": error: "
		 << "nature " << key << " is not declared." << endl;
	    error_count += 1;
	    return;
      }
}

void pform_discipline_flow(const struct vlltype&loc, const char*name)
{
      if (discipline_flow) {
	    cerr << loc.text << ":" << loc.first_line << ": error: "
		 << "Too many flow natures for discipline "
		 << discipline_name << "." << endl;
	    error_count += 1;
	    return;
      }

      perm_string key = lex_strings.make(name);
      discipline_flow = natures[key];

      if (discipline_flow == 0) {
	    cerr << loc.text << ":" << loc.first_line << ": error: "
		 << "nature " << key << " is not declared." << endl;
	    error_count += 1;
	    return;
      }
}

void pform_end_discipline(const struct vlltype&loc)
{
	// If the domain is not otherwise specified, then take it to
	// be continuous if potential or flow natures are given.
      if (discipline_domain == IVL_DIS_NONE && (discipline_potential||discipline_flow))
	    discipline_domain = IVL_DIS_CONTINUOUS;

      ivl_discipline_t tmp = new ivl_discipline_s(discipline_name,
						  discipline_domain,
						  discipline_potential,
						  discipline_flow);
      disciplines[discipline_name] = tmp;

      FILE_NAME(tmp, loc);

	/* Clear the static variables for the next item. */
      discipline_name = perm_string();
      discipline_domain = IVL_DIS_NONE;
      discipline_potential = 0;
      discipline_flow = 0;
}

/*
 * The parser uses this function to attach a discipline to a wire. The
 * wire may be declared by now, or will be declared further later. If
 * it is already declared, we just attach the discipline. If it is not
 * declared yet, then this is the declaration and we create the signal
 * in the current lexical scope.
 */
void pform_attach_discipline(const struct vlltype&loc,
			     ivl_discipline_t discipline, list<pform_ident_t>*names)
{
      for (list<pform_ident_t>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {

	    PWire* cur_net = pform_get_wire_in_scope(cur->first);
	    if (cur_net == 0) {
		    /* Not declared yet, declare it now. */
		  cur_net = pform_makewire(loc, *cur, NetNet::WIRE, 0);
		  ivl_assert(loc, cur_net);
	    }

	    if (ivl_discipline_t tmp = cur_net->get_discipline()) {
		  cerr << loc.text << ":" << loc.first_line << ": error: "
		       << "discipline " << discipline->name()
		       << " cannot override existing discipline " << tmp->name()
		       << " on net " << cur_net->basename() << endl;
		  error_count += 1;

	    } else {
		  data_type_t *type = new real_type_t(real_type_t::REAL);
		  FILE_NAME(type, loc);
		  cur_net->set_data_type(type);
		  cur_net->set_discipline(discipline);
	    }
      }
}
