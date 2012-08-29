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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "sys_priv.h"
# include  "stringheap.h"
# include  <string.h>
# include  <stdlib.h>
# include  <assert.h>
# include  "ivl_alloc.h"

struct stringheap_cell {
      struct stringheap_cell*next;
};

# define PAGE_SIZE 8192
# define STRINGHEAP_SIZE (PAGE_SIZE - sizeof(struct stringheap_cell))

const char*strdup_sh(struct stringheap_s*hp, const char*txt)
{
      char*res;
      unsigned len = strlen(txt);
      assert(len < STRINGHEAP_SIZE);

      if (hp->cell_lst == 0) {
	    hp->cell_lst = malloc(PAGE_SIZE);
	    hp->cell_lst->next = 0;
	    hp->cell_off = 0;
      }

      if ((STRINGHEAP_SIZE - hp->cell_off - 1) <= len) {
	    struct stringheap_cell*tmp = malloc(PAGE_SIZE);
	    tmp->next = hp->cell_lst;
	    hp->cell_lst = tmp;
	    hp->cell_off = 0;
      }

      res = (char*) (hp->cell_lst + 1);
      res += hp->cell_off;
      strcpy(res, txt);
      hp->cell_off += len + 1;

      return res;
}

void string_heap_delete(struct stringheap_s*hp)
{
      struct stringheap_cell *cur, *tmp;

      for (cur = hp->cell_lst; cur ; cur = tmp) {
	    tmp = cur->next;
	    free((char *)cur);
      }
      hp->cell_lst = 0;
      hp->cell_off = 0;
}
