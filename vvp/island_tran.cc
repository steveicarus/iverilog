/*
 * Copyright (c) 2008-2024 Stephen Williams (steve@icarus.com)
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

# include  "vvp_island.h"
# include  "compile.h"
# include  "symbols.h"
# include  "schedule.h"
# include  <list>

# include  <iostream>

using namespace std;

class vvp_island_tran : public vvp_island {

    public:
      void run_island();
      void count_drivers(vvp_island_port*port, unsigned bit_idx,
                         unsigned counts[3]);
};

enum tran_state_t {
      tran_disabled,
      tran_enabled,
      tran_unknown
};

struct vvp_island_branch_tran : public vvp_island_branch {

      vvp_island_branch_tran(vvp_net_t*en__, bool active_high__,
                             unsigned width__, unsigned part__,
                             unsigned offset__, bool resistive__);
      void run_test_enabled();
      bool rerun_test_enabled();
      void run_resolution();
      void run_output();

      vvp_net_t*en;
      unsigned width, part, offset;
      bool active_high, resistive;
      tran_state_t state;
};

vvp_island_branch_tran::vvp_island_branch_tran(vvp_net_t*en__,
                                               bool active_high__,
                                               unsigned width__,
                                               unsigned part__,
                                               unsigned offset__,
                                               bool resistive__)
: en(en__), width(width__), part(part__), offset(offset__),
  active_high(active_high__), resistive(resistive__)
{
      state = en__ ? tran_disabled : tran_enabled;
}

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
	// the results in the state for each branch.
      for (vvp_island_branch*cur = branches_ ; cur ; cur = cur->next_branch) {
	    vvp_island_branch_tran*tmp = dynamic_cast<vvp_island_branch_tran*>(cur);
	    assert(tmp);
	    tmp->run_test_enabled();
      }

rerun:
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

	// Now check if the enable inputs have been affected by the resolution.
      bool enable_changed = false;
      for (vvp_island_branch*cur = branches_ ; cur ; cur = cur->next_branch) {
	    vvp_island_branch_tran*tmp = dynamic_cast<vvp_island_branch_tran*>(cur);
	    assert(tmp);
	    enable_changed |= tmp->rerun_test_enabled();
      }
      if (enable_changed) goto rerun;
}

static void count_drivers_(vvp_branch_ptr_t cur, bool other_side_visited,
                           unsigned bit_idx, unsigned counts[3])
{
        // First count any value driven into the port associated with
        // the current endpoint.
      vvp_net_t*net = cur.port() ? cur.ptr()->b : cur.ptr()->a;
      vvp_scalar_t bit = island_get_value(net).value(bit_idx);
      update_driver_counts(bit.value(), counts);

        // Now handle all the branches attached to that port.
      vvp_branch_ptr_t idx = cur;
      do {
            vvp_island_branch_tran*tmp = BRANCH_TRAN(idx.ptr());

              // If this branch represents a tran gate, we count the
              // value on the other side of the tran (providing it is
              // enabled) as a single driver.
            if (tmp->width == 0) {
                  if (tmp->state == tran_enabled) {
                        net = idx.port() ? idx.ptr()->a : idx.ptr()->b;
                        bit = island_get_sent_value(net).value(bit_idx);
                        update_driver_counts(bit.value(), counts);
                  }
                  continue;
            }

              // If we get here, this branch is a part select. If we've
              // just come from the other end of the branch, we're done.
            if ((idx == cur) && other_side_visited)
                  continue;

              // If this is the narrow end of the part select, the other
              // end must include the bit we are interested in. Follow
              // the branch to collect any drivers on the other side.
            if (idx.port() == 1) {
                  vvp_branch_ptr_t a_side(tmp, 0);
                  count_drivers_(a_side, true, tmp->offset + bit_idx, counts);
                  continue;
            }

              // If we get here, this branch is the wide end of a part
              // select. If the bit we are interested in is within the
              // selected part, follow the branch to collect any drivers
              // on the other side.
            if ((bit_idx >= tmp->offset) && (bit_idx < tmp->offset+tmp->part)) {
                  vvp_branch_ptr_t b_side(tmp, 1);
                  count_drivers_(b_side, true, bit_idx - tmp->offset, counts);
                  continue;
            }
      } while ((idx = next(idx)) != cur);
}

void vvp_island_tran::count_drivers(vvp_island_port*port, unsigned bit_idx,
                                    unsigned counts[3])
{
        // First we need to find a branch that is attached to the specified
        // port. Unfortunately there's no quick way to do this.
      vvp_island_branch*branch = branches_;
      unsigned side = 0;
      while (branch) {
            if (branch->a->fun == port) {
                  side = 0;
                  break;
            }
            if (branch->b->fun == port) {
                  side = 1;
                  break;
            }
            branch = branch->next_branch;
      }
      assert(branch);

        // Now count the drivers, pushing through the network as necessary.
      vvp_branch_ptr_t endpoint(branch, side);
      count_drivers_(endpoint, false, bit_idx, counts);
}

void vvp_island_branch_tran::run_test_enabled()
{
      vvp_island_port*ep = en? dynamic_cast<vvp_island_port*> (en->fun) : NULL;

	// If there is no ep port (no "enabled" input) then this is a
	// tran branch. Assume it is always enabled.
      if (ep == 0) {
	    state = tran_enabled;
	    return;
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
      vvp_bit4_t enable_val;
      if (ep->outvalue.size() != 0)
	    enable_val = ep->outvalue.value(0).value();
      else if (ep->invalue.size() == 0)
	    enable_val = BIT4_Z;
      else
	    enable_val = ep->invalue.value(0).value();

      switch (enable_val) {
	  case BIT4_0:
	    state = active_high ? tran_disabled : tran_enabled;
	    break;
	  case BIT4_1:
	    state = active_high ? tran_enabled : tran_disabled;
	    break;
	  default:
	    state = tran_unknown;
	    break;
      }
}

bool vvp_island_branch_tran::rerun_test_enabled()
{
      vvp_island_port*ep = en? dynamic_cast<vvp_island_port*> (en->fun) : NULL;

      if (ep == 0)
	    return false;

	// We are only looking for changes resulting from running the island
	// resolution. If the outvalue is nil, we know that the enable port
	// is an .import, so won't be affected.
      if (ep->outvalue.size() == 0)
	    return false;

      vvp_bit4_t enable_val = ep->outvalue.value(0).value();

      tran_state_t old_state = state;

      switch (enable_val) {
	  case BIT4_0:
	    state = active_high ? tran_disabled : tran_enabled;
	    break;
	  case BIT4_1:
	    state = active_high ? tran_enabled : tran_disabled;
	    break;
	  default:
	    state = tran_unknown;
	    break;
      }

      return state != old_state;
}

// The IEEE standard does not specify the behaviour when a tranif control
// input is 'x' or 'z'. We use the rules that are given for MOS switches.
inline vvp_vector8_t resolve_ambiguous(const vvp_vector8_t&a,
                                       const vvp_vector8_t&b,
                                       tran_state_t state,
                                       unsigned str_map[8])
{
      assert(a.size() == b.size());
      vvp_vector8_t out (a.size());

      for (unsigned idx = 0 ;  idx < out.size() ;  idx += 1) {
	    vvp_scalar_t a_bit = a.value(idx);
	    vvp_scalar_t b_bit = b.value(idx);
            b_bit = vvp_scalar_t(b_bit.value(),
                                 str_map[b_bit.strength0()],
                                 str_map[b_bit.strength1()]);
	    if (state == tran_unknown) {
		  switch (b_bit.value()) {
		      case BIT4_0:
			b_bit = vvp_scalar_t(BIT4_X, b_bit.strength0(), 0);
			break;
		      case BIT4_1:
			b_bit = vvp_scalar_t(BIT4_X, 0, b_bit.strength1());
			break;
		      default:
			break;
		  }
	    }
	    out.set_bit(idx, resolve(a_bit, b_bit));
      }
      return out;
}

static void push_value_through_branches(const vvp_vector8_t&val,
					list<vvp_branch_ptr_t>&connections);

static void push_value_through_branch(const vvp_vector8_t&val,
                                      vvp_branch_ptr_t cur)
{
      vvp_island_branch_tran*branch = BRANCH_TRAN(cur.ptr());

        // If the branch is disabled, skip.
      if (branch->state == tran_disabled)
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

        // If we don't yet have an initial value for the port, skip.
      if (dst_port->value.size() == 0)
            return;

        // Now resolve the pushed value with whatever values we have
        // previously collected (and resolved) for the port.
      if (branch->width == 0) {
              // There are no part selects.
            dst_port->value = resolve_ambiguous(dst_port->value, val, branch->state,
                                                vvp_switch_strength_map[branch->resistive]);

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
	    port->value = vvp_vector8_t::nil;
      }

	// Do the same for the B side port.
      port = dynamic_cast<vvp_island_port*>(b->fun);
      if (port->value.size() != 0) {
	    island_send_value(b, port->value);
	    port->value = vvp_vector8_t::nil;
      }
}

void compile_island_tran(char*label)
{
      vvp_island*use_island = new vvp_island_tran;
      compile_island_base(label, use_island);
}

void compile_island_tranif(int sense, char*island, char*pa, char*pb, char*pe,
                           bool resistive)
{
      vvp_island*use_island = compile_find_island(island);
      assert(use_island);
      free(island);

      vvp_net_t*en = NULL;

      if (pe) {
	    en = use_island->find_port(pe);
	    assert(en);
	    free(pe);
      }

      vvp_island_branch_tran*br = new vvp_island_branch_tran(en,
                                                             sense ? true :
                                                                     false,
                                                             0, 0, 0, resistive);

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

      vvp_island_branch_tran*br = new vvp_island_branch_tran(NULL, false,
                                                             wid, par, off, false);

      use_island->add_branch(br, pa, pb);

      free(pa);
      free(pb);
}
