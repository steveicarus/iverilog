/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vcd_priv.c,v 1.6 2004/10/04 01:10:58 steve Exp $"
#endif

# include  "vpi_config.h"
# include  "vcd_priv.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <ctype.h>
# include  "stringheap.h"

int is_escaped_id(const char *name)
{
      int lp;

      assert(name);
        /* The first digit must be alpha or '_' to be a normal id. */
      if (isalpha(name[0]) || name[0] == '_') {
	    for (lp=1; name[lp] != '\0'; lp++) {
		    /* If this digit is not alpha-numeric or '_' we have
		     * an escaped identifier. */
		  if (!(isalnum(name[lp]) || name[lp] == '_')) {
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
      assert(nl);
      nl->name = strdup_sh(&name_heap, name);
      nl->next = tab->vcd_names_list;
      tab->vcd_names_list = nl;
      tab->listed_names ++;
}

static int vcd_names_compare(const void *s1, const void *s2)
{
      const char *v1 = *(const char **) s1;
      const char *v2 = *(const char **) s2;

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
	    assert(tab->vcd_names_sorted);

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
   Nexus Id cache

   In structural models, many signals refer to the same nexus.
   Some structural models also have very many signals.  This cache
   saves nexus_id - vcd_id pairs, and reuses the vcd_id when a signal
   refers to a nexus that is already dumped.

   The new signal will be listed as a $var, but no callback
   will be installed.  This saves considerable CPU time and leads
   to smalle VCD files.

   The _vpiNexusId is a private (int) property of IVL simulators.
*/

struct vcd_id_s
{
      const char *id;
      struct vcd_id_s *next;
      int nex;
};

static inline unsigned ihash(int nex)
{
      unsigned a = nex;
      a ^= a>>16;
      a ^= a>>8;
      return a & 0xff;
}

static struct vcd_id_s **vcd_ids = 0;

const char *find_nexus_ident(int nex)
{
      struct vcd_id_s *bucket;

      if (!vcd_ids) {
	    vcd_ids = (struct vcd_id_s **)
		  calloc(256, sizeof(struct vcd_id_s*));
	    assert(vcd_ids);
      }

      bucket = vcd_ids[ihash(nex)];
      while (bucket) {
	    if (nex == bucket->nex)
		  return bucket->id;
	    bucket = bucket->next;
      }

      return 0;
}

void set_nexus_ident(int nex, const char *id)
{
      struct vcd_id_s *bucket;

      assert(vcd_ids);

      bucket = (struct vcd_id_s *) malloc(sizeof(struct vcd_id_s));
      bucket->next = vcd_ids[ihash(nex)];
      bucket->id = id;
      bucket->nex = nex;
      vcd_ids[ihash(nex)] = bucket;
}

/* This is used by the compiletf routines to check if an argument
 * is numeric. */
static void check_numeric_arg(vpiHandle arg, char *msg, PLI_BYTE8 *name)
{
      assert(arg);

      switch (vpi_get(vpiType, arg)) {
         case vpiConstant:
         case vpiParameter:
             /* String constants are invalid numeric values. */
           if (vpi_get(vpiConstType, arg) == vpiStringConst) {
                 vpi_mcd_printf(1, msg, name);
                 vpi_control(vpiFinish, 1);
           }
           break;

           /* These have valid numeric values. */
         case vpiIntegerVar:
         case vpiMemoryWord:
         case vpiNet:
         case vpiRealVar:
         case vpiReg:
         case vpiTimeVar:
           break;

         default:
             /* Anything else is not a numeric value. */
           vpi_mcd_printf(1, msg, name);
           vpi_control(vpiFinish, 1);
           break;
      }
}

/* This is used by the compiletf routines to check if an argument
 * is a string value. */
static void check_string_arg(vpiHandle arg, char *msg, PLI_BYTE8 *name)
{
      assert(arg);
      PLI_INT32 ctype = 0;

      switch (vpi_get(vpiType, arg)) {
         case vpiConstant:
         case vpiParameter:
             /* These must be a string or binary constant. */
           ctype = vpi_get(vpiConstType, arg);
           if (ctype != vpiStringConst && ctype != vpiBinaryConst) {
                 vpi_mcd_printf(1, msg, name);
                 vpi_control(vpiFinish, 1);
           }
           break;

           /* These have valid string values. */
         case vpiIntegerVar:
         case vpiMemoryWord:
         case vpiNet:
         case vpiReg:
         case vpiTimeVar:
           break;

         default:
             /* Anything else is not a string. */
           vpi_mcd_printf(1, msg, name);
           vpi_control(vpiFinish, 1);
           break;
      }
}

/*
 * Since the compiletf routines are all the same they are located here,
 * so we only need a single copy.
 */

/* $dumpall does not take an argument. */
PLI_INT32 sys_dumpall_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      if (argv != 0) {
            vpi_mcd_printf(1, "ERROR: %s does not take an argument.\n", name);
            vpi_control(vpiFinish, 1);
      }

      return 0;
}

/* $dumpfile takes a single string argument. */
PLI_INT32 sys_dumpfile_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      /* Check that there is an argument and that it is a string. */
      if (argv == 0) {
            vpi_mcd_printf(1, "ERROR: %s requires an argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }
      check_string_arg(vpi_scan(argv), "ERROR: %s's argument must be a"
                                       " string.\n", name);

      /* Check that there is only a single argument. */
      if (vpi_scan(argv) != 0) {
            vpi_mcd_printf(1, "ERROR: %s takes a single argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      return 0;
}

/* $dumpflush does not take an argument. */
PLI_INT32 sys_dumpflush_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      if (argv != 0) {
            vpi_mcd_printf(1, "ERROR: %s does not take an argument.\n", name);
            vpi_control(vpiFinish, 1);
      }

      return 0;
}

/* $dumplimit takes a single numeric argument. */
PLI_INT32 sys_dumplimit_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      /* Check that there is an argument and that it is numeric. */
      if (argv == 0) {
            vpi_mcd_printf(1, "ERROR: %s requires an argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }
      check_numeric_arg(vpi_scan(argv), "ERROR: %s's argument must be"
                                        " numeric.\n", name);

      /* Check that there is only a single argument. */
      if (vpi_scan(argv) != 0) {
            vpi_mcd_printf(1, "ERROR: %s takes a single argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      return 0;
}

/* $dumpoff does not take an argument. */
PLI_INT32 sys_dumpoff_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      if (argv != 0) {
            vpi_mcd_printf(1, "ERROR: %s does not take an argument.\n", name);
            vpi_control(vpiFinish, 1);
      }

      return 0;
}

/* $dumpon does not take an argument. */
PLI_INT32 sys_dumpon_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      if (argv != 0) {
            vpi_mcd_printf(1, "ERROR: %s does not take an argument.\n", name);
            vpi_control(vpiFinish, 1);
      }

      return 0;
}

/* $dumpvars takes a variety of arguments. */
PLI_INT32 sys_dumpvars_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

      /* No arguments is OK, dump everything. */
      if (argv == 0)
	    return 0;

      /* The first argument is the numeric level. */
      check_numeric_arg(vpi_scan(argv), "ERROR: %s's first argument must be"
                                        " numeric.\n", name);

      /* The rest of the arguments are either a module or a variable. */
      while (arg = vpi_scan(argv)) {
        switch(vpi_get(vpiType, arg)) {
          /* The module types. */
          case vpiModule:
          case vpiTask:
          case vpiFunction:
          case vpiNamedBegin:
          case vpiNamedFork:
          /* The variable types. */
          case vpiNet:
          case vpiReg:
          case vpiMemoryWord:
          case vpiIntegerVar:
          case vpiTimeVar:
          case vpiRealVar:
            break;
          default:
            vpi_mcd_printf(1, "ERROR: %s cannot dump a %s.\n",
                           name, vpi_get_str(vpiType, arg));
            vpi_control(vpiFinish, 1);
        }
      }

      return 0;
}

