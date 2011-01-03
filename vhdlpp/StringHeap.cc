/*
 * Copyright (c) 2002-2010 Stephen Williams (steve@icarus.com)
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

# include  "StringHeap.h"
# include  <cstdlib>
# include  <cstring>
# include  <cassert>

#ifdef CHECK_WITH_VALGRIND
static char **string_pool = NULL;
static unsigned string_pool_count = 0;
#endif

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
#ifdef CHECK_WITH_VALGRIND
	    string_pool_count += 1;
	    string_pool = (char **) realloc(string_pool,
	                                    string_pool_count*sizeof(char **));
	    string_pool[string_pool_count-1] = cell_base_;
#endif
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

void StringHeapLex::cleanup()
{
#ifdef CHECK_WITH_VALGRIND
      for (unsigned idx = 0 ;  idx < string_pool_count ;  idx += 1) {
	    free(string_pool[idx]);
      }
      free(string_pool);
      string_pool = NULL;
      string_pool_count = 0;

      for (unsigned idx = 0 ;  idx < HASH_SIZE ;  idx += 1) {
	    hash_table_[idx] = 0;
      }
#endif
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

perm_string StringHeapLex::make(const string&text)
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
