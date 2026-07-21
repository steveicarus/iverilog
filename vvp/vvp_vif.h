#ifndef IVL_vvp_vif_H
#define IVL_vvp_vif_H
/*
 * Copyright (c) 2026 Stephen Williams (steve@icarus.com)
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

# include  "vvp_object.h"
# include  "vvp_net.h"
# include  <vector>

/*
 * A vvp_vif is the runtime representation of a SystemVerilog virtual
 * interface handle. Elaboration gives the virtual interface type the
 * IVL_VT_CLASS base type, so a virtual interface variable is a
 * .var/cobj-style variable and virtual interface handles are passed
 * around with the same %load/obj, %store/obj, %test_nul/obj, etc.,
 * opcodes used for class handles. vvp_vif is a vvp_object so that it
 * plugs directly into that machinery.
 *
 * A vvp_vif remembers, for each member signal of the interface, the
 * vvp_net_t that carries the member's value. It also lazily builds
 * edge-detector nets (posedge/negedge/anyedge) wired to those member
 * signals, so that %vif/wait can suspend a thread until a particular
 * member of a particular virtual interface handle changes.
 *
 * The signal nets are shared, elaboration-time objects (there is one
 * per member of the interface *instance*, not per virtual interface
 * handle) so many vvp_vif objects (e.g. many handles that were all
 * assigned from the same interface instance) end up pointing at the
 * same underlying nets. The edge-detector nets, however, are owned by
 * this vvp_vif and are created (at most once each) the first time
 * they are needed.
 */
class vvp_vif : public vvp_object {

    public:
      enum edge_t { EDGE_POSEDGE = 0, EDGE_NEGEDGE = 1, EDGE_ANYEDGE = 2 };

      explicit vvp_vif(size_t nmembers);
      ~vvp_vif() override;

      size_t size() const { return nmembers_; }

	// Bind member <idx> to the given signal net. This is called
	// once, right after construction, by %new/vif.
      void set_member(size_t idx, vvp_net_t*sig);

	// The net that carries the value of member <idx>.
      vvp_net_t* signal(size_t idx) const;

	// The edge-detector net for member <idx>/<edge>, created (and
	// wired to the member signal) the first time it is asked for.
      vvp_net_t* edge_event(size_t idx, edge_t edge);

    private:
      size_t nmembers_;
      std::vector<vvp_net_t*> signals_;
      std::vector<vvp_net_t*> pos_events_;
      std::vector<vvp_net_t*> neg_events_;
      std::vector<vvp_net_t*> any_events_;
};

#endif /* IVL_vvp_vif_H */
