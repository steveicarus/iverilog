/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
 *  ---
 *    You should also have recieved a copy of the Picture Elements
 *    Binary Software License offer along with the source. This offer
 *    allows you to obtain the right to redistribute the software in
 *    binary (compiled) form. If you have not received it, contact
 *    Picture Elements, Inc., 777 Panoramic Way, Berkeley, CA 94704.
 */
#if !defined(WINNT)
#ident "$Id: vvm_gates.cc,v 1.2 1999/11/24 04:38:49 steve Exp $"
#endif

# include  "vvm_gates.h"

vvm_out_event::vvm_out_event(vvm_simulation*s, vpip_bit_t v, action_t o)
: output_(o), sim_(s), val_(v)
{
}

vvm_out_event::~vvm_out_event()
{
}

void vvm_out_event::event_function()
{
      output_(sim_, val_);
}

vvm_1bit_out::vvm_1bit_out(vvm_out_event::action_t o, unsigned d)
: output_(o), delay_(d)
{
}

vvm_1bit_out::~vvm_1bit_out()
{
}

void vvm_1bit_out::output(vvm_simulation*sim, vpip_bit_t val)
{
      vvm_event*ev = new vvm_out_event(sim, val, output_);
      if (delay_ > 0)
	    sim->insert_event(delay_, ev);
      else
	    sim->active_event(ev);
}


vpip_bit_t compute_and(const vpip_bit_t*inp, unsigned count)
{
      vpip_bit_t outval = inp[0];
      for (unsigned i = 1 ;  i < count ;  i += 1)
	    outval = outval & inp[i];
      return outval;
}

vpip_bit_t compute_or(const vpip_bit_t*inp, unsigned count)
{
      vpip_bit_t outval = inp[0];
      for (unsigned i = 1 ;  i < count ;  i += 1)
	    outval = outval | inp[i];
      return outval;
}

vpip_bit_t compute_nor(const vpip_bit_t*inp, unsigned count)
{
      vpip_bit_t outval = inp[0];
      for (unsigned i = 1 ;  i < count ;  i += 1)
	    outval = outval | inp[i];
      return not(outval);
}

vpip_bit_t compute_xor(const vpip_bit_t*inp, unsigned count)
{
      vpip_bit_t outval = inp[0];
      for (unsigned i = 1; i < count; i++)
	    outval = outval ^ inp[i];
      return outval;
}

vpip_bit_t compute_nand(const vpip_bit_t*inp, unsigned count)
{
      return not(compute_and(inp,count));
}

vpip_bit_t compute_xnor(const vpip_bit_t*inp, unsigned count)
{
      return not(compute_xor(inp,count));
}


/*
 * $Log: vvm_gates.cc,v $
 * Revision 1.2  1999/11/24 04:38:49  steve
 *  LT and GT fixes from Eric Aardoom.
 *
 * Revision 1.1  1999/11/22 00:30:52  steve
 *  Detemplate some and, or and nor methods.
 *
 */

