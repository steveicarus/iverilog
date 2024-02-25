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
# include  "AStatement.h"

using namespace std;

AContrib* pform_contribution_statement(const struct vlltype&loc,
					 PExpr*lval, PExpr*rval)
{
      AContrib*tmp = new AContrib(lval, rval);
      FILE_NAME(tmp, loc);
      return tmp;
}

void pform_make_analog_behavior(const struct vlltype&loc, ivl_process_type_t pt,
				Statement*statement)
{
      AProcess*proc = new AProcess(pt, statement);
      FILE_NAME(proc, loc);

      pform_put_behavior_in_scope(proc);
}

PExpr* pform_make_branch_probe_expression(const struct vlltype&loc,
					  char*name, char*n1, char*n2)
{
      vector<named_pexpr_t> parms (2);
      parms[0].parm = new PEIdent(lex_strings.make(n1), loc.lexical_pos);
      FILE_NAME(parms[0].parm, loc);

      parms[1].parm = new PEIdent(lex_strings.make(n2), loc.lexical_pos);
      FILE_NAME(parms[1].parm, loc);

      PECallFunction*res = new PECallFunction(lex_strings.make(name), parms);
      FILE_NAME(res, loc);
      return res;
}

PExpr* pform_make_branch_probe_expression(const struct vlltype&loc,
					  char*name, char*branch_name)
{
      vector<named_pexpr_t> parms (1);
      parms[0].parm = new PEIdent(lex_strings.make(branch_name), loc.lexical_pos);
      FILE_NAME(parms[0].parm, loc);

      PECallFunction*res = new PECallFunction(lex_strings.make(name), parms);
      FILE_NAME(res, loc);

      return res;
}
