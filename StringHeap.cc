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
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: StringHeap.cc,v 1.4 2003/01/27 05:09:17 steve Exp $"
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
      cell_count_ = 0;
}

StringHeap::~StringHeap()
{
	// This is a planned memory leak. The string heap is intended
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
	    cell_count_ += 1;
	    assert(cell_base_ != 0);
      }

      char*res = cell_base_ + cell_ptr_;
      memcpy(res, text, len);
      cell_ptr_ += len;
      cell_base_[cell_ptr_++] = 0;

      assert(cell_ptr_ <= HEAPCELL);

      return res;
}

/*
 * $Log: StringHeap.cc,v $
 * Revision 1.4  2003/01/27 05:09:17  steve
 *  Spelling fixes.
 *
 * Revision 1.3  2003/01/16 21:44:46  steve
 *  Keep some debugging status.
 *
 * Revision 1.2  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/08/04 19:13:16  steve
 *  dll uses StringHeap for named items.
 *
 */

