/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
 *  ---
 *    You should also have recieved a copy of the Picture Elements
 *    Binary Software License offer along with the source. This offer
 *    allows you to obtain the right to redistribute the software in
 *    binary (compiled) form. If you have not received it, contact
 *    Picture Elements, Inc., 777 Panoramic Way, Berkeley, CA 94704.
 */
#if !defined(WINNT)
#ident "$Id: StringHeap.cc,v 1.1 2002/08/04 19:13:16 steve Exp $"
#endif

# include  "StringHeap.h"
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

StringHeap::StringHeap()
{
      cell_base_ = 0;
      cell_ptr_ = HEAPCELL;
}

StringHeap::~StringHeap()
{
	// This is a planned memory leak. The string heap is indended
	// to hold permanently-allocated strings.
}

const char* StringHeap::add(const char*text)
{
      unsigned len = strlen(text);
      assert((len+1) <= HEAPCELL);

      unsigned rem = HEAPCELL - cell_ptr_;
      if (rem < (len+1)) {
	    cell_base_ = (char*)malloc(HEAPCELL);
	    cell_ptr_ = 0;
      }

      char*res = cell_base_ + cell_ptr_;
      memcpy(res, text, len);
      cell_ptr_ += len;
      cell_base_[cell_ptr_++] = 0;

      return res;
}

/*
 * $Log: StringHeap.cc,v $
 * Revision 1.1  2002/08/04 19:13:16  steve
 *  dll uses StringHeap for named items.
 *
 */

