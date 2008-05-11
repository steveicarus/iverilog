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
 * The parser uses this function to attach a discipline to a declared wire.
 */
void pform_attach_discipline(const struct vlltype&loc,
			     discipline_t*discipline, list<perm_string>*names)
{
      error_count += 1;
      cerr << yylloc.text << ";" << yylloc.first_line << ": sorry: "
	   << "Net discipline declarations not supported." << endl;

      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; cur ++ ) {
	    cerr << yylloc.text << ";" << yylloc.first_line << ":      : "
		 << "discipline=" << discipline->name()
		 << ", net=" << *cur << endl;
      }
}

void pform_dump(std::ostream&out, discipline_t*dis)
{
      out << "discipline " << dis->name() << endl;
      out << "    domain " << dis->domain() << ";" << endl;
      out << "enddiscipline" << endl;
}

std::ostream& operator << (std::ostream&out, ddomain_t dom)
{
      switch (dom) {
	  case DD_NONE:
	    out << "no-domain";
	    break;
	  case DD_DISCRETE:
	    out << "discrete";
	    break;
	  case DD_CONTINUOUS:
	    out << "continuous";
	    break;
	  default:
	    assert(0);
	    break;
      }
      return out;
}
