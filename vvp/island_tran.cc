/*
 * Copyright (c) 2008,2011 Stephen Williams (steve@icarus.com)
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

      bool run_test_enabled();
      void run_resolution();
      void run_output();

      bool active_high;
      bool enabled_flag;
      vvp_net_t*en;
      unsigned width, part, offset;
};

static inline vvp_island_branch_tran* BRANCH_TRAN(vvp_island_branch*tmp)
{
      vvp_island_branch_tran*res = dynamic_cast<vvp_island_branch_tran*>(tmp);
      assert(res);
      return res;
}

/*
 * The run_island() method is called by the scheduler to run the
 * entire island. We run the island by calling run_resolution() for
 * all the branches in the island.
*/
void vvp_island_tran::run_island()
{
	// Test to see if any of the branches are enabled. This loop
	// tests the enabled inputs for all the branches and caches
	// the results in the enabled_flag for each branch.
      bool runnable = false;
      for (vvp_island_branch*cur = branches_ ; cur ; cur = cur->next_branch) {
	    vvp_island_branch_tran*tmp = dynamic_cast<vvp_island_branch_tran*>(cur);
	    assert(tmp);
	    runnable |= tmp->run_test_enabled();
      }

	// Now resolve all the branches in the island.
      for (vvp_island_branch*cur = branches_ ; cur ; cur = cur->next_branch) {
	    vvp_island_branch_tran*tmp = dynamic_cast<vvp_island_branch_tran*>(cur);
	    assert(tmp);
	    tmp->run_resolution();
      }

	// Now output the resolved values.
      for (vvp_island_branch*cur = branches_ ; cur ; cur = cur->next_branch) {
	    vvp_island_branch_tran*tmp = dynamic_cast<vvp_island_branch_tran*>(cur);
	    assert(tmp);
	    tmp->run_output();
      }
}

bool vvp_island_branch_tran::run_test_enabled()
{
      vvp_island_port*ep = en? dynamic_cast<vvp_island_port*> (en->fun) : 0;

	// If there is no ep port (no "enabled" input) then this is a
	// tran branch. Assume it is always enabled.
      if (ep == 0) {
	    enabled_flag = true;
	    return true;
      }

	// Get the input that is driving this enable.
	// SPECIAL NOTE: Try to get the input value from the
	// *outvalue* of the port. If the enable is connected to a
	// .port (instead of a .import) then there may be feedback
	// going on, and we need to be looking at the resolved input,
	// not the event input. For example:
	//
	//   tranif1 (pin, X, pin);
	//
	// In this case, when we test the value for "pin", we need to
	// look at the value that is resolved from this
	// island. Reading the outvalue will do the trick.
	//
	// If the outvalue is nil, then we know that this port is a
	// .import after all, so just read the invalue.
      enabled_flag = false;
      vvp_bit4_t enable_val;
      if (ep->outvalue.size() != 0)
	    enable_val = ep->outvalue.value(0).value();
      else if (ep->invalue.size() == 0)
	    enable_val = BIT4_Z;
      else
	    enable_val = ep->invalue.value(0).value();

      if (active_high==true && enable_val != BIT4_1)
	    return false;

      if (active_high==false && enable_val != BIT4_0)
	    return false;

      enabled_flag = true;
      return true;
}

static void push_value_through_branches(const vvp_vector8_t&val,
					list<vvp_branch_ptr_t>&connections);

static void push_value_through_branch(const vvp_vector8_t&val,
                                      vvp_branch_ptr_t cur)
{
      vvp_island_branch_tran*branch = BRANCH_TRAN(cur.ptr());

        // If the branch is not enabled, skip.
      if (! branch->enabled_flag)
            return;

      unsigned src_ab = cur.port();
      unsigned dst_ab = src_ab^1;

      vvp_net_t*dst_net = dst_ab? branch->b : branch->a;
      vvp_island_port*dst_port = dynamic_cast<vvp_island_port*>(dst_net->fun);

      vvp_vector8_t old_val = dst_port->value;

        // If the port on the other side has not yet been visited,
        // get its input value.
      if (dst_port->value.size() == 0)
            dst_port->value = island_get_value(dst_net);

	// If we don't yet have an initial value for this port, simply
	// derive the port value from the pushed value. This does not
	// need to be pushed back into the network.
      if (dst_port->value.size() == 0) {
	    if (branch->width == 0) {
		    // There are no part selects.
		  dst_port->value = val;
	    } else if (dst_ab == 1) {
		    // The other side is a strict subset (part select)
		    // of this side.
		  dst_port->value = val.subvalue(branch->offset, branch->part);
	    } else {
		   // The other side is a superset of this side.
		  dst_port->value = part_expand(val, branch->width,
					        branch->offset);
	    }
            return;
      }

        // Now resolve the pushed value with whatever values we have
        // previously collected (and resolved) for the port.
      if (branch->width == 0) {
              // There are no part selects.
            dst_port->value = resolve(dst_port->value, val);

      } else if (dst_ab == 1) {
              // The other side is a strict subset (part select)
              // of this side.
            vvp_vector8_t tmp = val.subvalue(branch->offset, branch->part);
            dst_port->value = resolve(dst_port->value, tmp);

      } else {
              // The other side is a superset of this side.
            vvp_vector8_t tmp = part_expand(val, branch->width, branch->offset);
            dst_port->value = resolve(dst_port->value, tmp);
      }

        // If the resolved value for the port has changed, push the new
        // value back into the network.
      if (! dst_port->value.eeq(old_val)) {
            list<vvp_branch_ptr_t> connections;

	    vvp_branch_ptr_t dst_side(branch, dst_ab);
	    island_collect_node(connections, dst_side);

	    push_value_through_branches(dst_port->value, connections);
      }
}

static void push_value_through_branches(const vvp_vector8_t&val,
					list<vvp_branch_ptr_t>&connections)
{
      for (list<vvp_branch_ptr_t>::iterator idx = connections.begin()
		 ; idx != connections.end() ; ++ idx ) {

            push_value_through_branch(val, *idx);
      }
}

/*
 * This method resolves the value for a branch recursively. It uses
 * recursive descent to span the graph of branches, pushing values
 * through the network until a stable state is reached.
 */
void vvp_island_branch_tran::run_resolution()
{
      list<vvp_branch_ptr_t> connections;
      vvp_island_port*port;

	// If the A side port hasn't already been visited, then push
        // its input value through all the branches connected to it.
      port = dynamic_cast<vvp_island_port*>(a->fun);
      if (port->value.size() == 0) {
	    vvp_branch_ptr_t a_side(this, 0);
	    island_collect_node(connections, a_side);

	    port->value = island_get_value(a);
            if (port->value.size() != 0)
	          push_value_through_branches(port->value, connections);

            connections.clear();
      }

	// Do the same for the B side port. Note that if the branch
        // is enabled, the B side port will have already been visited
        // when we resolved the A side port.
      port = dynamic_cast<vvp_island_port*>(b->fun);
      if (port->value.size() == 0) {
	    vvp_branch_ptr_t b_side(this, 1);
	    island_collect_node(connections, b_side);

	    port->value = island_get_value(b);
	    if (port->value.size() != 0)
	          push_value_through_branches(port->value, connections);

            connections.clear();
      }
}

void vvp_island_branch_tran::run_output()
{
      vvp_island_port*port;

	// If the A side port hasn't already been updated, send the
        // resolved value to the output.
      port = dynamic_cast<vvp_island_port*>(a->fun);
      if (port->value.size() != 0) {
	    island_send_value(a, port->value);
	    port->value = vvp_vector8_t();
      }

	// Do the same for the B side port.
      port = dynamic_cast<vvp_island_port*>(b->fun);
      if (port->value.size() != 0) {
	    island_send_value(b, port->value);
	    port->value = vvp_vector8_t();
      }
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
