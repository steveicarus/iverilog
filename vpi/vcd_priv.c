/*
 * Copyright (c) 2003-2021 Stephen Williams (steve@icarus.com)
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

#include  "sys_priv.h"
#include  "vcd_priv.h"
#include  "ivl_alloc.h"
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <assert.h>
#include  <ctype.h>
#include  "stringheap.h"

int is_escaped_id(const char *name)
{
      assert(name);
        /* The first digit must be alpha or '_' to be a normal id. */
      if (isalpha((int)name[0]) || name[0] == '_') {
	    int lp;
	    for (lp=1; name[lp] != '\0'; lp++) {
		    /* If this digit is not alpha-numeric or '_' we have
		     * an escaped identifier. */
		  if (!(isalnum((int)name[lp]) ||
		        name[lp] == '_') || name[lp] == '$') {
		        return 1;
		  }
	    }
	      /* We looked at all the digits, so this is a normal id. */
	    return 0;
      }
      return 1;
}

struct stringheap_s name_heap = {0, 0};

struct vcd_names_s {
      const char *name;
      struct vcd_names_s *next;
};

void vcd_names_add(struct vcd_names_list_s*tab, const char *name)
{
      struct vcd_names_s *nl = (struct vcd_names_s *)
	    malloc(sizeof(struct vcd_names_s));
      nl->name = strdup_sh(&name_heap, name);
      nl->next = tab->vcd_names_list;
      tab->vcd_names_list = nl;
      tab->listed_names ++;
}

void vcd_names_delete(struct vcd_names_list_s*tab)
{
      struct vcd_names_s *cur, *tmp;
      for (cur = tab->vcd_names_list; cur; cur = tmp) {
	    tmp = cur->next;
	    free(cur);
      }
      tab->vcd_names_list = 0;
      tab->listed_names = 0;
      free(tab->vcd_names_sorted);
      tab->vcd_names_sorted = 0;
      tab->sorted_names = 0;
      string_heap_delete(&name_heap);
}

static int vcd_names_compare(const void *s1, const void *s2)
{
      const char *v1 = *(const char * const *) s1;
      const char *v2 = *(const char * const *) s2;

      return strcmp(v1, v2);
}

const char *vcd_names_search(struct vcd_names_list_s*tab, const char *key)
{
      const char **v;

      if (tab->vcd_names_sorted == 0)
	    return 0;

      v = (const char **) bsearch(&key,
				  tab->vcd_names_sorted, tab->sorted_names,
				  sizeof(const char *), vcd_names_compare );

      return(v ? *v : NULL);
}

void vcd_names_sort(struct vcd_names_list_s*tab)
{
      if (tab->listed_names) {
	    struct vcd_names_s *r;
	    const char **l;

	    tab->sorted_names += tab->listed_names;
	    tab->vcd_names_sorted = (const char **)
		  realloc(tab->vcd_names_sorted,
			  tab->sorted_names*(sizeof(const char *)));

	    l = tab->vcd_names_sorted + tab->sorted_names - tab->listed_names;
	    tab->listed_names = 0;

	    r = tab->vcd_names_list;
	    tab->vcd_names_list = 0x0;

	    while (r) {
		  struct vcd_names_s *rr = r;
		  r = rr->next;
		  *(l++) = rr->name;
		  free(rr);
	    }

	    qsort(tab->vcd_names_sorted, tab->sorted_names,
		  sizeof(const char **), vcd_names_compare);
      }
}

/*
 * Since the compiletf routines are all the same they are located here,
 * so we only need a single copy. Some are generic enough they can use
 * the ones in sys_priv.c (no arg, one numeric argument, etc.).
 */

/* $dumpvars takes a variety of arguments. */
PLI_INT32 sys_dumpvars_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

      /* No arguments is OK, dump everything. */
      if (argv == 0) return 0;

      /* The first argument is the numeric level. */
      if (! is_numeric_obj(vpi_scan(argv))) {
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s's argument must be numeric.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
      }

      /* The rest of the arguments are either a module or a variable. */
      while ((arg=vpi_scan(argv)) != NULL) {
        switch(vpi_get(vpiType, arg)) {
          case vpiMemoryWord:
/*
 * We need to allow non-constant selects to support the following:
 *
 * for (lp = 0; lp < max ; lp = lp + 1) $dumpvars(0, array[lp]);
 *
 * We need to do a direct callback on the selected element vs using
 * the &A<> structure. The later will not give us what we want.
 * This is implemented in the calltf routine.
 */
#if 0
            if (vpi_get(vpiConstantSelect, arg) == 0) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s cannot dump a non-constant select %s.\n", name,
		             vpi_get_str(vpiType, arg));
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
            }
#endif
          /* The module types. */
          case vpiModule:
          case vpiGenScope:
          case vpiFunction:
          case vpiTask:
          case vpiNamedBegin:
          case vpiNamedFork:
          /* The variable types. */
#if 0
          case vpiParameter: /* A constant! */
#endif
          case vpiNet:
          case vpiReg:
          case vpiIntegerVar:
          case vpiBitVar:
          case vpiByteVar:
          case vpiShortIntVar:
          case vpiIntVar:
          case vpiLongIntVar:
          case vpiTimeVar:
          case vpiRealVar:
          case vpiNamedEvent:
            break;

          case vpiParameter: /* A constant! */
            vpi_printf("SORRY: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s cannot currently dump a parameter.\n", name);
            break;

          default:
            vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("%s cannot dump a %s.\n", name,
                       vpi_get_str(vpiType, arg));
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
        }
      }

      return 0;
}
