/*
 * Copyright (c) 2008 Stephen Williams (steve@icarus.com)
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

# include  "vvp_island.h"
# include  "compile.h"
# include  "symbols.h"
# include  "schedule.h"
# include  <list>

using namespace std;

class vvp_island_tran : public vvp_island {

    public:
      void run_island();
};

struct vvp_island_branch_tran : public vvp_island_branch {

	// Behavior. (This stuff should be moved to a derived
	// class. The members here are specific to the tran island
	// class.)
      bool run_test_enabled();
      void run_resolution();
      bool active_high;
      bool enabled_flag;
      vvp_net_t*en;
      int flags;
      unsigned width, part, offset;
};

static inline vvp_island_branch_tran* BRANCH_TRAN(vvp_island_branch*tmp)
{
      vvp_island_branch_tran*res = dynamic_cast<vvp_island_branch_tran*>(tmp);
      assert(res);
      return res;
}

void vvp_island_tran::run_island()
{
	// Test to see if any of the branches are enabled.
      bool runnable = false;
      for (vvp_island_branch*cur = branches_ ; cur ; cur = cur->next_branch) {
	    vvp_island_branch_tran*tmp = dynamic_cast<vvp_island_branch_tran*>(cur);
	    assert(tmp);
	    runnable |= tmp->run_test_enabled();
      }
      if (runnable == false)
	    return;

      for (vvp_island_branch*cur = branches_ ; cur ; cur = cur->next_branch) {
	    vvp_island_branch_tran*tmp = dynamic_cast<vvp_island_branch_tran*>(cur);
	    assert(tmp);
	    tmp->run_resolution();
      }
}

bool vvp_island_branch_tran::run_test_enabled()
{
      flags = 0;

      vvp_island_port*ep = en? dynamic_cast<vvp_island_port*> (en->fun) : 0;

	// If there is no ep port (no "enabled" input) then this is a
	// tran branch. Assume it is always enabled.
      if (ep == 0) {
	    enabled_flag = true;
	    return true;
      }

      enabled_flag = false;
      vvp_bit4_t enable_val = ep->invalue.value(0).value();

      if (active_high==true && enable_val != BIT4_1)
	    return false;

      if (active_high==false && enable_val != BIT4_0)
	    return false;

      enabled_flag = true;
      return true;
}

static void mark_done_flags(list<vvp_branch_ptr_t>&connections)
{
      for (list<vvp_branch_ptr_t>::iterator idx = connections.begin()
		 ; idx != connections.end() ; idx ++ ) {

	    vvp_island_branch*tmp_ptr = idx->ptr();
	    vvp_island_branch_tran*cur = dynamic_cast<vvp_island_branch_tran*>(tmp_ptr);

	    unsigned tmp_ab = idx->port();
	    cur->flags |= 1 << tmp_ab;
      }
}

static void mark_visited_flags(list<vvp_branch_ptr_t>&connections)
{
      for (list<vvp_branch_ptr_t>::iterator idx = connections.begin()
		 ; idx != connections.end() ; idx ++ ) {

	    vvp_island_branch*tmp_ptr = idx->ptr();
	    vvp_island_branch_tran*cur = dynamic_cast<vvp_island_branch_tran*>(tmp_ptr);
	    assert(cur);

	    unsigned tmp_ab = idx->port();
	    cur->flags |= 4 << tmp_ab;
      }
}

static void clear_visited_flags(list<vvp_branch_ptr_t>&connections)
{
      for (list<vvp_branch_ptr_t>::iterator idx = connections.begin()
		 ; idx != connections.end() ; idx ++ ) {

	    vvp_island_branch_tran*tmp_ptr = BRANCH_TRAN(idx->ptr());

	    unsigned tmp_ab = idx->port();
	    tmp_ptr->flags &= ~(4 << tmp_ab);
      }
}

static vvp_vector8_t get_value_from_branch(vvp_branch_ptr_t cur);

static void resolve_values_from_connections(vvp_vector8_t&val,
					    list<vvp_branch_ptr_t>&connections)
{
      for (list<vvp_branch_ptr_t>::iterator idx = connections.begin()
		 ; idx != connections.end() ; idx ++ ) {
	    vvp_vector8_t tmp = get_value_from_branch(*idx);
	    if (val.size() == 0)
		  val = tmp;
	    else if (tmp.size() != 0)
		  val = resolve(val, tmp);
      }
}

static vvp_vector8_t get_value_from_branch(vvp_branch_ptr_t cur)
{
      vvp_island_branch_tran*ptr = BRANCH_TRAN(cur.ptr());
      assert(ptr);
      unsigned ab = cur.port();
      unsigned ab_other = ab^1;

	// If the branch link is disabled, return nil.
      if (ptr->enabled_flag == false)
	    return vvp_vector8_t();

      vvp_branch_ptr_t other (ptr, ab_other);

	// If the branch other side is already visited, return nil.
      if (ptr->flags & (4<<ab_other))
	    return vvp_vector8_t();

	// Other side net, and port value.
      vvp_net_t*net_other = ab? ptr->a : ptr->b;
      vvp_vector8_t val_other = island_get_value(net_other);

	// recurse
      list<vvp_branch_ptr_t> connections;
      island_collect_node(connections, other);
      mark_visited_flags(connections);

      resolve_values_from_connections(val_other, connections);

	// Remove visited flag
      clear_visited_flags(connections);

      if (val_other.size() == 0)
	    return val_other;

      if (ptr->width) {
	    if (ab == 0) {
		  val_other = part_expand(val_other, ptr->width, ptr->offset);

	    } else {
		  val_other = val_other.subvalue(ptr->offset, ptr->part);

	    }
      }

      return val_other;
}

static void push_value_through_branches(const vvp_vector8_t&val,
					list<vvp_branch_ptr_t>&connections)
{
      for (list<vvp_branch_ptr_t>::iterator idx = connections.begin()
		 ; idx != connections.end() ; idx ++ ) {

	    vvp_island_branch_tran*tmp_ptr = BRANCH_TRAN(idx->ptr());
	    unsigned tmp_ab = idx->port();
	    unsigned other_ab = tmp_ab^1;

	      // If other side already done, skip
	    if (tmp_ptr->flags & (1<<other_ab))
		  continue;

	      // If link is not enabled, skip.
	    if (! tmp_ptr->enabled_flag)
		  continue;

	    vvp_net_t*other_net = other_ab? tmp_ptr->b : tmp_ptr->a;

	    if (tmp_ptr->width == 0) {
		    // Mark this end as done
		  tmp_ptr->flags |= (1 << other_ab);
		  island_send_value(other_net, val);

	    } if (other_ab == 1) {
		    // Mark as done
		  tmp_ptr->flags |= (1 << other_ab);
		  vvp_vector8_t tmp = val.subvalue(tmp_ptr->offset, tmp_ptr->part);
		  island_send_value(other_net, tmp);
	    } else {
		    // Otherwise, the other side is not fully
		    // specified, so we can't take this shortcut.
	    }
      }
}

void vvp_island_branch_tran::run_resolution()
{
	// Collect all the branch endpoints that are joined to my A
	// side.
      list<vvp_branch_ptr_t> connections;
      bool processed_a_side = false;
      vvp_vector8_t val;

      if ((flags & 1) == 0) {
	    processed_a_side = true;
	    vvp_branch_ptr_t a_side(this, 0);
	    island_collect_node(connections, a_side);

	      // Mark my A side as done. Do this early to prevent recursing
	      // back. All the connections that share this port are also
	      // done. Make sure their flags are set appropriately.
	    mark_done_flags(connections);

	    val = island_get_value(a);
	    mark_visited_flags(connections); // Mark as visited.


	      // Now scan the other sides of all the branches connected to
	      // my A side. The get_value_from_branch() will recurse as
	      // necessary to depth-first walk the graph.
	    resolve_values_from_connections(val, connections);

	      // A side is done.
	    island_send_value(a, val);

	      // Clear the visited flags. This must be done so that other
	      // branches can read this input value.
	    clear_visited_flags(connections);

	      // Try to push the calculated value out through the
	      // branches. This is useful for A-side results because
	      // there is a high probability that the other side of
	      // all the connected branches is fully specified by this
	      // result.
	    push_value_through_branches(val, connections);
      }

	// If the B side got taken care of by above, then this branch
	// is done. Stop now.
      if (flags & 2)
	    return;

	// Repeat the above for the B side.

      connections.clear();
      island_collect_node(connections, vvp_branch_ptr_t(this, 1));
      mark_done_flags(connections);

      if (enabled_flag && processed_a_side) {
	      // If this is a connected branch, then we know from the
	      // start that we have all the bits needed to complete
	      // the B side. Even if the B side is a part select, the
	      // simple part select must be correct because the
	      // recursive resolve_values_from_connections above must
	      // of cycled back to the B side of myself when resolving
	      // the connections.
	    if (width != 0)
		  val = val.subvalue(offset, part);

      } else {

	      // If this branch is not enabled, then the B-side must
	      // be processed on its own.
	    val = island_get_value(b);
	    mark_visited_flags(connections);
	    resolve_values_from_connections(val, connections);
	    clear_visited_flags(connections);
      }

      island_send_value(b, val);
}

void compile_island_tran(char*label)
{
      vvp_island*use_island = new vvp_island_tran;
      compile_island_base(label, use_island);
}

void compile_island_tranif(int sense, char*island, char*pa, char*pb, char*pe)
{
      vvp_island*use_island = compile_find_island(island);
      assert(use_island);
      free(island);

      vvp_island_branch_tran*br = new vvp_island_branch_tran;
      if (sense)
	    br->active_high = true;
      else
	    br->active_high = false;

      if (pe == 0) {
	    br->en = 0;
      } else {
	    br->en = use_island->find_port(pe);
	    assert(br->en);
	    free(pe);
      }

      br->width = 0;
      br->part = 0;
      br->offset = 0;

      use_island->add_branch(br, pa, pb);

      free(pa);
      free(pb);
}


void compile_island_tranvp(char*island, char*pa, char*pb,
			   unsigned wid, unsigned par, unsigned off)
{
      vvp_island*use_island = compile_find_island(island);
      assert(use_island);
      free(island);

      vvp_island_branch_tran*br = new vvp_island_branch_tran;
      br->active_high = false;
      br->en = 0;
      br->width = wid;
      br->part = par;
      br->offset = off;

      use_island->add_branch(br, pa, pb);

      free(pa);
      free(pb);
}
