/*
 * Copyright (c) 2008 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "compiler.h"
# include  "pform.h"
# include  "parse_misc.h"
# include  "discipline.h"

map<perm_string,discipline_t*> disciplines;

static perm_string discipline_name;
static ddomain_t discipline_domain = DD_NONE;

void pform_start_discipline(const char*name)
{
      discipline_name = lex_strings.make(name);
      discipline_domain = DD_NONE;
}

void pform_end_discipline(const struct vlltype&loc)
{
      discipline_t*tmp = new discipline_t(discipline_name, discipline_domain);
      disciplines[discipline_name] = tmp;

      FILE_NAME(tmp, loc);

	/* Clear the static variables for the next item. */
      discipline_name = perm_string::perm_string();
      discipline_domain = DD_NONE;
}

/*
 * The parser uses this function to attach a discipline to a wire. The
 * wire may be declared by now, or will be declared further later. If
 * it is already declared, we just attach the discipline. If it is not
 * declared yet, then this is the declaration and we create the signal
 * in the current lexical scope.
 */
void pform_attach_discipline(const struct vlltype&loc,
			     discipline_t*discipline, list<perm_string>*names)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; cur ++ ) {

	    PWire* cur_net = pform_get_wire_in_scope(*cur);
	    if (cur_net == 0) {
		    /* Not declared yet, declare it now. */
		  pform_makewire(loc, *cur, NetNet::WIRE,
				 NetNet::NOT_A_PORT, IVL_VT_REAL, 0);
		  cur_net = pform_get_wire_in_scope(*cur);
		  assert(cur_net);
	    }

	    if (discipline_t*tmp = cur_net->get_discipline()) {
		  cerr << loc.text << ":" << loc.first_line << ": error: "
		       << "discipline " << discipline->name()
		       << " cannot override existing discipline " << tmp->name()
		       << " on net " << cur_net->basename() << endl;
		  error_count += 1;

	    } else {
		  cur_net->set_discipline(discipline);
	    }
      }
}
