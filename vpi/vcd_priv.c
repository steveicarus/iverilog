/*
 * Copyright (c) 2003-2010 Stephen Williams (steve@icarus.com)
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

# include  "vpi_config.h"
# include  "vcd_priv.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  "stringheap.h"

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
   to smaller VCD files.

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
