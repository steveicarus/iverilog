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
#ident "$Id: stringheap.c,v 1.3 2004/01/21 01:22:53 steve Exp $"
#endif

# include  "sys_priv.h"
# include  "stringheap.h"
# include  <string.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>

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

/*
 * $Log: stringheap.c,v $
 * Revision 1.3  2004/01/21 01:22:53  steve
 *  Give the vip directory its own configure and vpi_config.h
 *
 * Revision 1.2  2003/04/28 01:03:11  steve
 *  Fix stringheap list management failure.
 *
 * Revision 1.1  2003/02/13 18:13:28  steve
 *  Make lxt use stringheap to perm-allocate strings.
 *
 */

