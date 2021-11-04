/*
 * Copyright (c) 2016-2021 Martin Whitaker (icarus@martin-whitaker.me.uk)
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

# include "config.h"

# include  <cstdlib>
# include  <sstream>
# include  "netlist.h"
# include  "functor.h"
# include  "compiler.h"
# include  "ivl_assert.h"

using namespace std;

/*
 * The exposenodes functor is primarily provided for use by the vlog95
 * target. To implement some LPM objects, it needs to take a bit or part
 * of one of the LPM inputs. If that input is not connected to a real
 * net in the design, we need to create a net at that point so that
 * there is something to which we can apply a bit or part select. This
 * has the effect of splitting the synthesised structure at that point.
 * Rather than creating a new net, we just look for a temporary net
 * created by the synthesis process (there should be at least one) and
 * reset its "local" flag. We also prepend another '_' to the synthetic
 * name to avoid name collisions when we recompile the vlog95 output
 * (because NetScope::local_symbol() doesn't actually check that the
 * name it generates is unique).
 */

struct exposenodes_functor  : public functor_t {

      unsigned count;

      virtual void lpm_mux(Design*des, NetMux*obj);
      virtual void lpm_part_select(Design*des, NetPartSelect*obj);
      virtual void lpm_substitute(Design*des, NetSubstitute*obj);
};

static bool expose_nexus(Nexus*nex)
{
      NetNet*sig = 0;
      for (Link*cur = nex->first_nlink() ; cur ; cur = cur->next_nlink()) {

	      // Don't expose nodes that are attached to constants
	    if (dynamic_cast<NetConst*> (cur->get_obj()))
		  return false;
	    if (dynamic_cast<NetLiteral*> (cur->get_obj()))
		  return false;

	    NetNet*cur_sig = dynamic_cast<NetNet*> (cur->get_obj());
	    if (cur_sig == 0)
		  continue;

	    if (!cur_sig->local_flag())
		  return false;

	    sig = cur_sig;
      }
      assert(sig);
      ostringstream res;
      res << "_" << sig->name();
      sig->rename(lex_strings.make(res.str()));
      sig->local_flag(false);
      return true;
}

/*
 * The vlog95 target implements a wide mux as a hierarchy of 2:1 muxes,
 * picking off one bit of the select input at each level of the hierarchy.
 */
void exposenodes_functor::lpm_mux(Design*, NetMux*obj)
{
      if (obj->sel_width() == 1)
	    return;

      if (expose_nexus(obj->pin_Sel().nexus()))
	    count += 1;
}

/*
 * A VP part select is going to select a part from its input.
 */
void exposenodes_functor::lpm_part_select(Design*, NetPartSelect*obj)
{
      if (obj->dir() != NetPartSelect::VP)
	    return;

      if (expose_nexus(obj->pin(1).nexus()))
	    count += 1;
}

/*
 * A substitute is going to select one or two parts from the wider input signal.
 */
void exposenodes_functor::lpm_substitute(Design*, NetSubstitute*obj)
{
      if (expose_nexus(obj->pin(1).nexus()))
	    count += 1;
}

void exposenodes(Design*des)
{
      exposenodes_functor exposenodes;
      exposenodes.count = 0;
      if (verbose_flag) {
	    cout << " ... Look for intermediate nodes" << endl << flush;
      }
      des->functor(&exposenodes);
      if (verbose_flag) {
	    cout << " ... Exposed " << exposenodes.count
		 << " intermediate signals." << endl << flush;
      }
}
