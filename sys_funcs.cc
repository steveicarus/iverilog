/*
 * Copyright (c) 2004-2019 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "compiler.h"
# include  <cstdio>
# include  <cstring>
# include  <cstdlib>

/*
 * Manage the information about system functions. This information is
 * collected from the sources before elaboration and made available
 * via the lookup_sys_func function.
 */

static const struct sfunc_return_type default_return_type =
    { 0, IVL_VT_LOGIC, 32, false, false };

struct sfunc_return_type_cell : sfunc_return_type {
      struct sfunc_return_type_cell*next;
};

static struct sfunc_return_type_cell*sfunc_list_head = 0;
static struct sfunc_return_type_cell*sfunc_list_tail = 0;

void append_to_list(struct sfunc_return_type_cell*cell)
{
      if (sfunc_list_tail) {
	    sfunc_list_tail->next = cell;
	    sfunc_list_tail = cell;
      } else {
	    sfunc_list_head = cell;
	    sfunc_list_tail = cell;
      }
      cell->next = 0;
}

void cleanup_sys_func_table()
{
      struct sfunc_return_type_cell *next, *cur = sfunc_list_head;
      while (cur) {
	    next = cur->next;
	    delete cur;
	    cur = next;
      }
}

static struct sfunc_return_type* find_in_sys_func_list(const char*name)
{
      struct sfunc_return_type_cell*cur = sfunc_list_head;
      while (cur) {
	    if (strcmp(cur->name, name) == 0)
		  return cur;

	    cur = cur->next;
      }

      return 0;
}

const struct sfunc_return_type* lookup_sys_func(const char*name)
{
	/* First, try to find the name in the function list. */
      struct sfunc_return_type*def = find_in_sys_func_list(name);
      if (def)
	    return def;

	/* No luck finding, so return the default description. */
      return &default_return_type;
}

void add_sys_func(const struct sfunc_return_type&ret_type)
{
      struct sfunc_return_type*def = find_in_sys_func_list(ret_type.name);
      if (def) {
              /* Keep the original definition, but flag that it
                 overrides a later definition. */
            def->override_flag = true;
            return;
      }
      struct sfunc_return_type_cell*cell = new struct sfunc_return_type_cell;
      cell->name = lex_strings.add(ret_type.name);
      cell->type = ret_type.type;
      cell->wid  = ret_type.wid;
      cell->signed_flag = ret_type.signed_flag;
      cell->override_flag = ret_type.override_flag;
      append_to_list(cell);
}

/*
 * This function loads a system functions descriptor file with the
 * format:
 *
 *    <name> <type> [<arguments>]
 *
 * The driver passes us user-provided tables first, so we add new entries
 * to the end of the list. This allows user-defined functions to override
 * built-in functions.
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

            struct sfunc_return_type*def = find_in_sys_func_list(name);
            if (def) {
                    /* Keep the original definition, but flag that it
                       overrides a later definition. */
                  def->override_flag = true;
                  continue;
            }

	    if (strcmp(stype,"vpiSysFuncReal") == 0) {
		  cell = new struct sfunc_return_type_cell;
		  cell->name = lex_strings.add(name);
		  cell->type = IVL_VT_REAL;
		  cell->wid  = 1;
		  cell->signed_flag = true;
		  cell->override_flag = false;
		  append_to_list(cell);
		  continue;
	    }

	    if (strcmp(stype,"vpiSysFuncInt") == 0) {
		  cell = new struct sfunc_return_type_cell;
		  cell->name = lex_strings.add(name);
		  cell->type = IVL_VT_LOGIC;
		  cell->wid  = 32;
		  cell->signed_flag = true;
		  cell->override_flag = false;
		  append_to_list(cell);
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
		  cell->type = IVL_VT_LOGIC;
		  cell->wid  = width;
		  cell->signed_flag = signed_flag;
		  cell->override_flag = false;
		  append_to_list(cell);
		  continue;
	    }

	    if (strcmp(stype,"vpiSysFuncVoid") == 0) {
		  cell = new struct sfunc_return_type_cell;
		  cell->name = lex_strings.add(name);
		  cell->type = IVL_VT_VOID;
		  cell->wid  = 0;
		  cell->signed_flag = false;
		  cell->override_flag = false;
		  append_to_list(cell);
		  continue;
	    }

	    if (strcmp(stype,"vpiSysFuncString") == 0) {
		  cell = new struct sfunc_return_type_cell;
		  cell->name = lex_strings.add(name);
		  cell->type = IVL_VT_STRING;
		  cell->wid  = 0;   // string is a dynamic length type
		  cell->signed_flag = false;
		  cell->override_flag = false;
		  append_to_list(cell);
		  continue;
	    }

	    fprintf(stderr, "%s:%s: Unknown type: %s\n",
		    path, name, stype);
      }
      fclose(fd);

      return 0;
}
