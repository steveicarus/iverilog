/*
 * Copyright (c) 2002-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: StringHeap.cc,v 1.6.2.1 2005/08/13 00:45:53 steve Exp $"
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

perm_string StringHeap::make(const char*text)
{
      return perm_string(add(text));
}


StringHeapLex::StringHeapLex()
{
      hit_count_ = 0;
      add_count_ = 0;

      for (unsigned idx = 0 ;  idx < HASH_SIZE ;  idx += 1)
	    hash_table_[idx] = 0;
}

StringHeapLex::~StringHeapLex()
{
}

unsigned StringHeapLex::add_hit_count() const
{
      return hit_count_;
}

unsigned StringHeapLex::add_count() const
{
      return add_count_;
}

static unsigned hash_string(const char*text)
{
      unsigned h = 0;

      while (*text) {
	    h = (h << 4) ^ (h >> 28) ^ *text;
	    text += 1;
      }
      return h;
}

const char* StringHeapLex::add(const char*text)
{
      unsigned hash_value = hash_string(text) % HASH_SIZE;

	/* If we easily find the string in the hash table, then return
	   that and be done. */
      if (hash_table_[hash_value]
	  && (strcmp(hash_table_[hash_value], text) == 0)) {
	    hit_count_ += 1;
	    return hash_table_[hash_value];
      }

	/* The existing hash entry is not a match. Replace it with the
	   newly allocated value, and return the new pointer as the
	   result to the add. */
      const char*res = StringHeap::add(text);
      hash_table_[hash_value] = res;
      add_count_ += 1;

      return res;
}

perm_string StringHeapLex::make(const char*text)
{
      return perm_string(add(text));
}

perm_string StringHeapLex::make(const std::string&text)
{
      return perm_string(add(text.c_str()));
}

bool operator == (perm_string a, const char*b)
{
      if (a.str() == b)
	    return true;

      if (! (a.str() && b))
	    return false;

      if (strcmp(a.str(), b) == 0)
	    return true;

      return false;
}

bool operator == (perm_string a, perm_string b)
{
      return a == b.str();
}

bool operator != (perm_string a, const char*b)
{
      return ! (a == b);
}

bool operator != (perm_string a, perm_string b)
{
      return ! (a == b);
}

bool operator < (perm_string a, perm_string b)
{
      if (b.str() && !a.str())
	    return true;

      if (b.str() == a.str())
	    return false;

      if (strcmp(a.str(), b.str()) < 0)
	    return true;

      return false;
}

/*
 * $Log: StringHeap.cc,v $
 * Revision 1.6.2.1  2005/08/13 00:45:53  steve
 *  Fix compilation warnings/errors with newer compilers.
 *
 * Revision 1.6  2004/02/18 17:11:54  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.5  2003/03/01 06:25:30  steve
 *  Add the lex_strings string handler, and put
 *  scope names and system task/function names
 *  into this table. Also, permallocate event
 *  names from the beginning.
 *
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

