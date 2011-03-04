/*
 * Copyright (c) 1998-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  <iostream>

# include  "functor.h"
# include  "netlist.h"
# include  "netmisc.h"

class xnfio_f  : public functor_t {

    public:
      void signal(Design*des, NetNet*sig);
      void lpm_compare(Design*des, NetCompare*dev);

    private:
      bool compare_sideb_const(Design*des, NetCompare*dev);
};

static bool is_a_pad(const NetNet*net)
{
      if (net->attribute(perm_string::literal("PAD")) == verinum())
	    return false;

      return true;
}

/*
 * The xnfio function looks for the PAD signals in the design, and
 * generates the needed IOB devices to handle being connected to the
 * actual FPGA PAD. This will add items to the netlist if needed.
 *
 * FIXME: If there is a DFF connected to the pad, try to convert it
 *        to an IO DFF instead. This would save a CLB, and it is
 *        really lame to not do the obvious optimization.
 */

static NetLogic* make_obuf(Design*des, NetNet*net)
{
      NetScope* scope = net->scope();
      assert(scope);

      assert(net->pin_count() == 1);

	/* FIXME: If there is nothing internally driving this PAD, I
	   can connect the PAD to a pullup and disconnect it from the
	   rest of the circuit. This would save routing resources. */
      if (count_outputs(net->pin(0)) <= 0) {
	    cerr << net->get_line() << ":warning: No outputs to OPAD: "
		 << net->name() << endl;
	    return 0;
      }

      assert(count_outputs(net->pin(0)) > 0);

	/* Look for an existing OBUF connected to this signal. If it
	   is there, then no need to add one. */
      Nexus*nex = net->pin(0).nexus();
      for (Link*idx = nex->first_nlink()
		 ; idx  ; idx = idx->next_nlink()) {
	    NetLogic*tmp;
	    if ((tmp = dynamic_cast<NetLogic*>(idx->get_obj())) == 0)
		  continue;

	      // Try to use an existing BUF as an OBUF. This moves the
	      // BUF into the IOB.
	    if ((tmp->type() == NetLogic::BUF)
		&& (count_inputs(tmp->pin(0)) == 0)
		&& (count_outputs(tmp->pin(0)) == 1)
		&& (idx->get_pin() == 0)  ) {
		  tmp->attribute(perm_string::literal("XNF-LCA"),
				 verinum("OBUF:O,I"));
		  return tmp;
	    }

	      // Try to use an existing INV as an OBUF. Certain
	      // technologies support inverting the input of an OBUF,
	      // which looks just like an inverter. This uses the
	      // available resources of an IOB to optimize away an
	      // otherwise expensive inverter.
	    if ((tmp->type() == NetLogic::NOT)
		&& (count_inputs(tmp->pin(0)) == 0)
		&& (count_outputs(tmp->pin(0)) == 1)
		&& (idx->get_pin() == 0)  ) {
		  tmp->attribute(perm_string::literal("XNF-LCA"),
				 verinum("OBUF:O,~I"));
		  return tmp;
	    }

	      // Try to use an existing bufif1 as an OBUFT. Of course
	      // this will only work if the output of the bufif1 is
	      // connected only to the pad. Handle bufif0 the same
	      // way, but the T input is inverted.
	    if ((tmp->type() == NetLogic::BUFIF1)
		&& (count_inputs(tmp->pin(0)) == 0)
		&& (count_outputs(tmp->pin(0)) == 1)
		&& (idx->get_pin() == 0)  ) {
		  tmp->attribute(perm_string::literal("XNF-LCA"),
				 verinum("OBUFT:O,I,~T"));
		  return tmp;
	    }

	    if ((tmp->type() == NetLogic::BUFIF0)
		&& (count_inputs(tmp->pin(0)) == 0)
		&& (count_outputs(tmp->pin(0)) == 1)
		&& (idx->get_pin() == 0)  ) {
		  tmp->attribute(perm_string::literal("XNF-LCA"),
				 verinum("OBUFT:O,I,T"));
		  return tmp;
	    }
      }

	// Can't seem to find a way to rearrange the existing netlist,
	// so I am stuck creating a new buffer, the OBUF.
      NetLogic*buf = new NetLogic(scope, scope->local_symbol(),
				  2, NetLogic::BUF);
      des->add_node(buf);

      buf->attribute(perm_string::literal("XNF-LCA"), verinum("OBUF:O,I"));

	// Put the buffer between this signal and the rest of the
	// netlist.
      connect(net->pin(0), buf->pin(1));
      net->pin(0).unlink();
      connect(net->pin(0), buf->pin(0));

	// It is possible, in putting an OBUF between net and the rest
	// of the netlist, to create a ring without a signal. Detect
	// this case and create a new signal.
      if (count_signals(buf->pin(1)) == 0) {
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE);
	    tmp->local_flag(true);
	    connect(buf->pin(1), tmp->pin(0));
      }

      return buf;
}

static void absorb_OFF(Design*des, NetLogic*buf)
{
	/* If the nexus connects is not a simple point-to-point link,
	   then I can't drag it into the IOB. Give up. */
      if (count_outputs(buf->pin(1)) != 1)
	    return;
      if (count_inputs(buf->pin(1)) != 1)
	    return;
	/* For now, only support OUTFF. */
      if (buf->type() != NetLogic::BUF)
	    return;

      Link*drv = find_next_output(&buf->pin(1));
      assert(drv);

	/* Make sure the device is a FF with width 1. */
      NetFF*ff = dynamic_cast<NetFF*>(drv->get_obj());
      if (ff == 0)
	    return;
      if (ff->width() != 1)
	    return;
      if (ff->attribute(perm_string::literal("LPM_FFType")) != verinum("DFF"))
	    return;

	/* Connect the flip-flop output to the buffer output and
	   delete the buffer. The XNF OUTFF can buffer the pin. */
      connect(ff->pin_Q(0), buf->pin(0));
      delete buf;

	/* Finally, build up an XNF-LCA value that defines this
	   devices as an OUTFF and gives each pin an XNF name. */
      const char**names = new const char*[ff->pin_count()];
      for (unsigned idx = 0 ;  idx < ff->pin_count() ;  idx += 1)
	    names[idx] = "";

      if (ff->attribute(perm_string::literal("Clock:LPM_Polarity")) == verinum("INVERT"))
	    names[ff->pin_Clock().get_pin()] = "~C";
      else
	    names[ff->pin_Clock().get_pin()] = "C";

      names[ff->pin_Data(0).get_pin()] = "D";
      names[ff->pin_Q(0).get_pin()] = "Q";

      string lname = string("OUTFF:") + names[0];
      for (unsigned idx = 1 ;  idx < ff->pin_count() ;  idx += 1)
	    lname = lname + "," + names[idx];
      delete[]names;

      ff->attribute(perm_string::literal("XNF-LCA"), lname);
}

static void make_ibuf(Design*des, NetNet*net)
{
      NetScope*scope = net->scope();
      assert(scope);

      assert(net->pin_count() == 1);
	// XXXX For now, require at least one input.
      assert(count_inputs(net->pin(0)) > 0);

	/* Look for an existing BUF connected to this signal and
	   suitably connected that I can use it as an IBUF. */

      Nexus*nex = net->pin(0).nexus();
      for (Link*idx = nex->first_nlink()
		 ; idx ; idx = idx->next_nlink()) {
	    NetLogic*tmp;
	    if ((tmp = dynamic_cast<NetLogic*>(idx->get_obj())) == 0)
		  continue;

	    if (tmp->attribute(perm_string::literal("XNF-LCA")) != verinum())
		  continue;

	      // Found a BUF, it is only usable if the only input is
	      // the signal and there are no other inputs.
	    if ((tmp->type() == NetLogic::BUF) &&
		(count_inputs(tmp->pin(1)) == 1) &&
		(count_outputs(tmp->pin(1)) == 0)) {
		  tmp->attribute(perm_string::literal("XNF-LCA"), verinum("IBUF:O,I"));
		  return;
	    }

      }

	// I give up, create an IBUF.
      NetLogic*buf = new NetLogic(scope, scope->local_symbol(),
				  2, NetLogic::BUF);
      des->add_node(buf);

      buf->attribute(perm_string::literal("XNF-LCA"), verinum("IBUF:O,I"));

	// Put the buffer between this signal and the rest of the
	// netlist.
      connect(net->pin(0), buf->pin(0));
      net->pin(0).unlink();
      connect(net->pin(0), buf->pin(1));

	// It is possible, in putting an OBUF between net and the rest
	// of the netlist, to create a ring without a signal. Detect
	// this case and create a new signal.
      if (count_signals(buf->pin(0)) == 0) {
	    NetNet*tmp = new NetNet(scope,
				    scope->local_symbol(),
				    NetNet::WIRE);
	    connect(buf->pin(0), tmp->pin(0));
      }
}

void xnfio_f::signal(Design*des, NetNet*net)
{
      if (! is_a_pad(net))
	    return;

      assert(net->pin_count() == 1);
      string pattr = net->attribute(perm_string::literal("PAD")).as_string();

      switch (pattr[0]) {
	  case 'i':
	  case 'I':
	    make_ibuf(des, net);
	    break;
	  case 'o':
	  case 'O': {
		NetLogic*buf = make_obuf(des, net);
		if (buf == 0) break;
		absorb_OFF(des, buf);
		break;
	  }

	      // FIXME: Only IPAD and OPAD supported. Need to
	      // add support for IOPAD.
	  default:
	    assert(0);
	    break;
      }
}

/*
 * Attempt some XNF specific optimizations on comparators.
 */
void xnfio_f::lpm_compare(Design*des, NetCompare*dev)
{
      if (compare_sideb_const(des, dev))
	    return;

      return;
}

bool xnfio_f::compare_sideb_const(Design*des, NetCompare*dev)
{
	/* Even if side B is all constant, if there are more than 4
	   signals on side A we will not be able to fit the operation
	   into a function unit, so we might as well accept a
	   comparator. Give up. */
      if (dev->width() > 4)
	    return false;

      NetScope*scope = dev->scope();

      verinum side (verinum::V0, dev->width());

	/* Is the B side all constant? */
      for (unsigned idx = 0 ;  idx < dev->width() ;  idx += 1) {

	    if (! dev->pin_DataB(idx).nexus()->drivers_constant())
		  return false;

	    side.set(idx, dev->pin_DataB(idx).nexus()->driven_value());
      }

	/* Handle the special case of comparing A to 0. Use an N-input
	   NOR gate to return 0 if any of the bits is not 0. */
      if ((side.as_ulong() == 0) && (count_inputs(dev->pin_AEB()) > 0)) {
	    NetLogic*sub = new NetLogic(scope, dev->name(), dev->width()+1,
					NetLogic::NOR);
	    connect(sub->pin(0), dev->pin_AEB());
	    for (unsigned idx = 0 ;  idx < dev->width() ;  idx += 1)
		  connect(sub->pin(idx+1), dev->pin_DataA(idx));
	    delete dev;
	    des->add_node(sub);
	    return true;
      }

	/* Handle the special case of comparing A to 0. Use an N-input
	   NOR gate to return 0 if any of the bits is not 0. */
      if ((side.as_ulong() == 0) && (count_inputs(dev->pin_ANEB()) > 0)) {
	    NetLogic*sub = new NetLogic(scope, dev->name(), dev->width()+1,
					NetLogic::OR);
	    connect(sub->pin(0), dev->pin_ANEB());
	    for (unsigned idx = 0 ;  idx < dev->width() ;  idx += 1)
		  connect(sub->pin(idx+1), dev->pin_DataA(idx));
	    delete dev;
	    des->add_node(sub);
	    return true;
      }

      return false;
}

void xnfio(Design*des)
{
      xnfio_f xnfio_obj;
      des->functor(&xnfio_obj);
}
