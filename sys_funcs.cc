/*
 * Copyright (c) 2004 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: sys_funcs.cc,v 1.6 2004/10/04 01:10:55 steve Exp $"
#endif

# include  "config.h"
# include  "compiler.h"
# include  <stdio.h>

/*
 * Manage the information about system functions. This information is
 * collected from the sources before elaboration and made available
 * via the lookup_sys_func function.
 */

static const struct sfunc_return_type sfunc_table[] = {
      { "$realtime",   NetExpr::ET_REAL,    0, 0 },
      { "$bitstoreal", NetExpr::ET_REAL,    0, 0 },
      { "$itor",       NetExpr::ET_REAL,    0, 0 },
      { "$realtobits", NetExpr::ET_VECTOR, 64, 0 },
      { "$time",       NetExpr::ET_VECTOR, 64, 0 },
      { "$stime",      NetExpr::ET_VECTOR, 32, 0 },
      { "$simtime",    NetExpr::ET_VECTOR, 64, 0 },
      { 0,             NetExpr::ET_VECTOR, 32, 0 }
};

struct sfunc_return_type_cell : sfunc_return_type {
      struct sfunc_return_type_cell*next;
};

static struct sfunc_return_type_cell*sfunc_stack = 0;

const struct sfunc_return_type* lookup_sys_func(const char*name)
{
	/* First, try to find then name in the function stack. */
      struct sfunc_return_type_cell*cur = sfunc_stack;
      while (cur) {
	    if (strcmp(cur->name, name) == 0)
		  return cur;

	    cur = cur->next;
      }

	/* Next, look in the core table. */
      unsigned idx = 0;
      while (sfunc_table[idx].name) {

	    if (strcmp(sfunc_table[idx].name, name) == 0)
		  return sfunc_table + idx;

	    idx += 1;
      }

	/* No luck finding, so return the trailer, which give a
	   default description. */
      return sfunc_table + idx;
}

/*
 * This function loads a system functions descriptor file with the
 * format:
 *
 *    <name> <type> [<arguments>]
 */
int load_sys_func_table(const char*path)
{
      struct sfunc_return_type_cell*cell;
      FILE*fd = fopen(path, "r");

      if (fd == 0) {
	    if (verbose_flag) {
		  fprintf(stderr, "%s: Unable to open System Function Table file.\n", path);
	    }
	    return -1;
      }

      if (verbose_flag) {
	    fprintf(stderr, "%s: Processing System Function Table file.\n", path);
      }

      char buf[256];
      while (fgets(buf, sizeof buf, fd)) {
	    char*name = buf + strspn(buf, " \t\r\n");

	      /* Skip empty lines. */
	    if (name[0] == 0)
		  continue;
	      /* Skip comment lines. */
	    if (name[0] == '#')
		  continue;

	    char*cp = name + strcspn(name, " \t\r\n");
	    if (cp[0]) *cp++ = 0;

	    cp += strspn(cp, " \t\r\n");

	    char*stype = cp;
	    if (stype[0] == 0) {
		  fprintf(stderr, "%s:%s: No function type?\n",
			  path, name);
		  continue;
	    }

	    cp = stype + strcspn(stype, " \t\r\n");
	    if (cp[0]) *cp++ = 0;

	    if (strcmp(stype,"vpiSysFuncReal") == 0) {
		  cell = new struct sfunc_return_type_cell;
		  cell->name = lex_strings.add(name);
		  cell->type = NetExpr::ET_REAL;
		  cell->wid  = 0;
		  cell->signed_flag = true;
		  cell->next = sfunc_stack;
		  sfunc_stack = cell;
		  continue;
	    }

	    if (strcmp(stype,"vpiSysFuncInt") == 0) {
		  cell = new struct sfunc_return_type_cell;
		  cell->name = lex_strings.add(name);
		  cell->type = NetExpr::ET_VECTOR;
		  cell->wid  = 32;
		  cell->signed_flag = true;
		  cell->next = sfunc_stack;
		  sfunc_stack = cell;
		  continue;
	    }

	      /* If this is a sized integer, then parse the additional
		 arguments, the width (decimal) and the optional
		 signed/unsigned flag. */
	    if (strcmp(stype,"vpiSysFuncSized") == 0) {
		  cp += strspn(cp, " \t\r\n");
		  char*swidth = cp;
		  unsigned width = 32;
		  bool signed_flag = false;

		  cp = swidth + strcspn(swidth, " \t\r\n");
		  if (cp[0]) *cp++ = 0;

		  width = strtoul(swidth, 0, 10);

		  cp += strspn(cp, " \t\r\n");
		  char*flag = cp;

		  while (flag[0]) {
			cp = flag + strcspn(flag, " \t\r\n");
			if (cp[0]) *cp++ = 0;

			if (strcmp(flag,"signed") == 0) {
			      signed_flag = true;

			} else if (strcmp(flag,"unsigned") == 0) {
			      signed_flag = false;
			}

			flag = cp + strspn(cp, " \t\r\n");
		  }

		  cell = new struct sfunc_return_type_cell;
		  cell->name = lex_strings.add(name);
		  cell->type = NetExpr::ET_VECTOR;
		  cell->wid  = width;
		  cell->signed_flag = signed_flag;
		  cell->next = sfunc_stack;
		  sfunc_stack = cell;
		  continue;
	    }

	    fprintf(stderr, "%s:%s: Unknown type: %s\n",
		    path, name, stype);
      }

      return 0;
}

/*
 * $Log: sys_funcs.cc,v $
 * Revision 1.6  2004/10/04 01:10:55  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.5  2004/03/17 17:07:12  steve
 *  Protect ident pragma.
 *
 * Revision 1.4  2004/03/13 05:12:36  steve
 *  standard conpliant stdio.h.
 *
 * Revision 1.3  2004/03/11 06:02:58  steve
 *  Verbose details for sft parsing.
 *
 * Revision 1.2  2004/03/10 04:51:24  steve
 *  Add support for system function table files.
 *
 * Revision 1.1  2004/03/09 04:29:42  steve
 *  Separate out the lookup_sys_func table, for eventual
 *  support for function type tables.
 *
 *  Remove ipal compile flags.
 */

