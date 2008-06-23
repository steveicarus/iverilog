#ifndef __slab_H
#define __slab_H
/*
 * Copyright (c) 2008 Picture Elements, Inc.
 *    Stephen Williams (steve@icarus.com)
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


template <size_t SLAB_SIZE, size_t CHUNK_COUNT> class slab_t {

      union item_cell_u {
	    item_cell_u*next;
	    char space[SLAB_SIZE];
      };

    public:
      slab_t();

      void* alloc_slab();
      void  free_slab(void*);

      unsigned long pool;

    private:
      item_cell_u*heap_;
      item_cell_u initial_chunk_[CHUNK_COUNT];
};

template <size_t SLAB_SIZE, size_t CHUNK_COUNT>
slab_t<SLAB_SIZE,CHUNK_COUNT>::slab_t()
{
      pool = CHUNK_COUNT;
      heap_ = initial_chunk_;
      for (unsigned idx = 0 ; idx < CHUNK_COUNT-1 ; idx += 1)
	    initial_chunk_[idx].next = initial_chunk_+idx+1;

      initial_chunk_[CHUNK_COUNT-1].next = 0;
}

template <size_t SLAB_SIZE, size_t CHUNK_COUNT>
inline void* slab_t<SLAB_SIZE,CHUNK_COUNT>::alloc_slab()
{
      if (heap_ == 0) {
	    item_cell_u*chunk = new item_cell_u[CHUNK_COUNT];
	    for (unsigned idx = 0 ; idx < CHUNK_COUNT ; idx += 1) {
		  chunk[idx].next = heap_;
		  heap_ = chunk+idx;
	    }
	    pool += CHUNK_COUNT;
      }

      item_cell_u*cur = heap_;
      heap_ = heap_->next;
      return cur;
}

template <size_t SLAB_SIZE, size_t CHUNK_COUNT>
inline void slab_t<SLAB_SIZE,CHUNK_COUNT>::free_slab(void*ptr)
{
      item_cell_u*cur = reinterpret_cast<item_cell_u*> (ptr);
      cur->next = heap_;
      heap_ = cur;
}

#endif
