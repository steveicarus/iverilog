#ifndef IVL_stringheap_H
#define IVL_stringheap_H
/*
 * Copyright (c) 2003-2014 Stephen Williams (steve@icarus.com)
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

struct stringheap_cell;

struct stringheap_s {
      struct stringheap_cell*cell_lst;
      unsigned cell_off;
};

/*
 * Allocate the string from the heap.
 */
const char*strdup_sh(struct stringheap_s*hp, const char*str);

void string_heap_delete(struct stringheap_s*hp);

#endif /* IVL_stringheap_H */
