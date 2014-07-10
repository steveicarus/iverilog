/*
 * Copyright (c) 2010-2011 Stephen Williams (steve@icarus.com)
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

# include  "vcd_priv.h"
# include  <map>
# include  <set>
# include  <string>
# include  <pthread.h>
# include  <cstdlib>
# include  <cstring>
# include  <cassert>

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

static std::map<int,const char*> nexus_ident_map;

extern "C" const char*find_nexus_ident(int nex)
{
      std::map<int,const char*>::const_iterator cur = nexus_ident_map.find(nex);
      if (cur == nexus_ident_map.end())
	    return 0;
      else
	    return cur->second;
}

extern "C" void set_nexus_ident(int nex, const char*id)
{
      nexus_ident_map[nex] = id;
}

extern "C" void nexus_ident_delete()
{
      nexus_ident_map.clear();
}


static std::set<std::string> vcd_scope_names_set;

extern "C" void vcd_scope_names_add(const char*name)
{
      vcd_scope_names_set .insert(name);
}

extern "C" int vcd_scope_names_test(const char*name)
{
      if (vcd_scope_names_set.find(name) == vcd_scope_names_set.end())
	    return 0;
      else
	    return 1;
}

extern "C" void vcd_scope_names_delete(void)
{
      vcd_scope_names_set.clear();
}

static pthread_t work_thread;

static const unsigned WORK_QUEUE_SIZE = 128*1024;
static const unsigned WORK_QUEUE_BATCH_MIN = 4*1024;
static const unsigned WORK_QUEUE_BATCH_MAX = 32*1024;

static struct vcd_work_item_s work_queue[WORK_QUEUE_SIZE];
static volatile unsigned work_queue_next = 0;
static volatile unsigned work_queue_fill = 0;

static pthread_mutex_t work_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  work_queue_is_empty_sig = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  work_queue_notempty_sig = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  work_queue_minfree_sig = PTHREAD_COND_INITIALIZER;


extern "C" struct vcd_work_item_s* vcd_work_thread_peek(void)
{
	// There must always only be 1 vcd work thread, and only the
	// work thread decreases the fill, so if the work_queue_fill
	// is non-zero, I can reliably assume that there is at least
	// one item that I can peek at. I only need to lock if I must
	// wait for the work_queue_fill to become non-zero.
      if (work_queue_fill == 0) {
	    pthread_mutex_lock(&work_queue_mutex);
	    while (work_queue_fill == 0)
		  pthread_cond_wait(&work_queue_notempty_sig, &work_queue_mutex);
	    pthread_mutex_unlock(&work_queue_mutex);
      }

      return work_queue + work_queue_next;
}

extern "C" void vcd_work_thread_pop(void)
{
      pthread_mutex_lock(&work_queue_mutex);

      unsigned use_fill = work_queue_fill - 1;
      work_queue_fill = use_fill;

      unsigned use_next = work_queue_next;

      struct vcd_work_item_s*cell = work_queue + use_next;
      if (cell->type == WT_EMIT_BITS) {
	    free(cell->op_.val_char);
      }

      use_next += 1;
      if (use_next >= WORK_QUEUE_SIZE)
	    use_next = 0;
      work_queue_next = use_next;

      if (use_fill == WORK_QUEUE_SIZE-WORK_QUEUE_BATCH_MIN)
	    pthread_cond_signal(&work_queue_minfree_sig);
      else if (use_fill == 0)
	    pthread_cond_signal(&work_queue_is_empty_sig);

      pthread_mutex_unlock(&work_queue_mutex);
}

/*
 * Work queue items are created in batches to reduce thread
 * bouncing. When the producer gets a free work item, it actually
 * locks the queue in order to produce a batch. The queue stays locked
 * until the batch is complete. Then the releases the whole lot to the
 * consumer.
 */
static uint64_t work_queue_next_time = 0;
static unsigned current_batch_cnt = 0;
static unsigned current_batch_alloc = 0;
static unsigned current_batch_base = 0;

extern "C" void vcd_work_start( void* (*fun) (void*), void*arg )
{
      pthread_create(&work_thread, 0, fun, arg);
}

static struct vcd_work_item_s* grab_item(void)
{
      if (current_batch_alloc == 0) {
	     pthread_mutex_lock(&work_queue_mutex);
	     while ((WORK_QUEUE_SIZE-work_queue_fill) < WORK_QUEUE_BATCH_MIN)
		  pthread_cond_wait(&work_queue_minfree_sig, &work_queue_mutex);

	     current_batch_base = work_queue_next + work_queue_fill;
	     current_batch_alloc = WORK_QUEUE_SIZE - work_queue_fill;

	     pthread_mutex_unlock(&work_queue_mutex);

	     if (current_batch_base >= WORK_QUEUE_SIZE)
		   current_batch_base -= WORK_QUEUE_SIZE;
	     if (current_batch_alloc > WORK_QUEUE_BATCH_MAX)
		   current_batch_alloc = WORK_QUEUE_BATCH_MAX;
	     current_batch_cnt = 0;
      }

      assert(current_batch_cnt < current_batch_alloc);

      unsigned cur = current_batch_base + current_batch_cnt;
      if (cur >= WORK_QUEUE_SIZE)
	    cur -= WORK_QUEUE_SIZE;

	// Write the new timestamp into the work item.
      struct vcd_work_item_s*cell = work_queue + cur;
      cell->time = work_queue_next_time;
      return cell;
}

static void end_batch(void)
{
      pthread_mutex_lock(&work_queue_mutex);

      unsigned use_fill = work_queue_fill;
      bool was_empty_flag = (use_fill==0) && (current_batch_cnt > 0);

      use_fill += current_batch_cnt;
      work_queue_fill = use_fill;

      current_batch_alloc = 0;
      current_batch_cnt = 0;

      if (was_empty_flag)
	    pthread_cond_signal(&work_queue_notempty_sig);

      pthread_mutex_unlock(&work_queue_mutex);
}

static inline void unlock_item(bool flush_batch =false)
{
      current_batch_cnt += 1;
      if (current_batch_cnt == current_batch_alloc || flush_batch)
	    end_batch();
}

extern "C" void vcd_work_sync(void)
{
      if (current_batch_alloc > 0)
	    end_batch();

      if (work_queue_fill > 0) {
	    pthread_mutex_lock(&work_queue_mutex);
	    while (work_queue_fill > 0)
		  pthread_cond_wait(&work_queue_is_empty_sig, &work_queue_mutex);
	    pthread_mutex_unlock(&work_queue_mutex);
      }
}

extern "C" void vcd_work_flush(void)
{
      struct vcd_work_item_s*cell = grab_item();
      cell->type = WT_FLUSH;
      unlock_item(true);
}

extern "C" void vcd_work_dumpon(void)
{
      struct vcd_work_item_s*cell = grab_item();
      cell->type = WT_DUMPON;
      unlock_item();
}

extern "C" void vcd_work_dumpoff(void)
{
      struct vcd_work_item_s*cell = grab_item();
      cell->type = WT_DUMPOFF;
      unlock_item();
}

extern "C" void vcd_work_set_time(uint64_t val)
{
      work_queue_next_time = val;
}

extern "C" void vcd_work_emit_double(struct lxt2_wr_symbol*sym, double val)
{
      struct vcd_work_item_s*cell = grab_item();
      cell->type = WT_EMIT_DOUBLE;
      cell->sym_.lxt2 = sym;
      cell->op_.val_double = val;
      unlock_item();
}

extern "C" void vcd_work_emit_bits(struct lxt2_wr_symbol*sym, const char* val)
{

      struct vcd_work_item_s*cell = grab_item();
      cell->type = WT_EMIT_BITS;
      cell->sym_.lxt2 = sym;
      cell->op_.val_char = strdup(val);

      unlock_item();
}

extern "C" void vcd_work_terminate(void)
{
      struct vcd_work_item_s*cell = grab_item();
      cell->type = WT_TERMINATE;
      unlock_item(true);
      pthread_join(work_thread, 0);
}
