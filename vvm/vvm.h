#ifndef __vvm_H
#define __vvm_H
/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vvm.h,v 1.37 2001/01/16 02:44:18 steve Exp $"
#endif

# include  <cassert>
# include  "vpi_priv.h"

/*
 * The Verilog Virtual Machine are definitions for the virtual machine
 * that executes models that the simulation generator makes.
 */

typedef unsigned vvm_u32;

class vvm_event;
class vvm_thread;


inline vpip_bit_t B_AND(vpip_bit_t l, vpip_bit_t r)
{
      if (B_IS0(l)) return St0;
      if (B_IS0(r)) return St0;
      if (B_IS1(l) && B_IS1(r)) return St1;
      return StX;
}

inline vpip_bit_t B_OR(vpip_bit_t l, vpip_bit_t r)
{
      if (B_IS1(l)) return St1;
      if (B_IS1(r)) return St1;
      if (B_IS0(l) && B_IS0(r)) return St0;
      return StX;
}

inline vpip_bit_t B_XOR(vpip_bit_t l, vpip_bit_t r)
{
      if (B_ISZ(l)) return StX;
      if (B_ISZ(r)) return StX;
      if (B_ISX(l)) return StX;
      if (B_ISX(r)) return StX;
      if (B_IS0(l)) return r;
      return B_IS0(r)? St1 : St0;
}

inline vpip_bit_t less_with_cascade(vpip_bit_t l, vpip_bit_t r, vpip_bit_t c)
{
      if (B_ISXZ(l)) return StX;
      if (B_ISXZ(r)) return StX;
      if ((l&0x80) > (r&0x80)) return St0;
      if ((l&0x80) < (r&0x80)) return St1;
      return c;
}

inline vpip_bit_t greater_with_cascade(vpip_bit_t l, vpip_bit_t r, vpip_bit_t c)
{
      if (B_ISXZ(l)) return StX;
      if (B_ISXZ(r)) return StX;
      if ((l&0x80) > (r&0x80)) return St1;
      if ((l&0x80) < (r&0x80)) return St0;
      return c;
}

extern vpip_bit_t add_with_carry(vpip_bit_t l, vpip_bit_t r, vpip_bit_t&carry);

inline vpip_bit_t B_NOT(vpip_bit_t l)
{
      if (B_IS0(l)) return St1;
      if (B_IS1(l)) return St0;
      return StX;
}

/*
 * These functions return true if the transition from A to B is a
 * Verilog type of negedge of posedge.
 */
extern bool negedge(vpip_bit_t from, vpip_bit_t to);
extern bool posedge(vpip_bit_t from, vpip_bit_t to);

/*
 * Verilog events (update events and nonblocking assign) are derived
 * from this abstract class so that the simulation engine can treat
 * all of them identically.
 */
class vvm_event {

    public:
      vvm_event();
      virtual ~vvm_event() =0;
      virtual void event_function() =0;

      void schedule(unsigned long delay =0);

    private:
      struct vpip_event*event_;

      static void callback_(void*);

    private: // not implemented
      vvm_event(const vvm_event&);
      vvm_event& operator= (const vvm_event&);
};


/*
 * $Log: vvm.h,v $
 * Revision 1.37  2001/01/16 02:44:18  steve
 *  Use the iosfwd header if available.
 *
 * Revision 1.36  2000/04/10 05:26:06  steve
 *  All events now use the NetEvent class.
 *
 * Revision 1.35  2000/03/26 16:55:41  steve
 *  Remove the vvm_bits_t abstract class.
 *
 * Revision 1.34  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.33  2000/03/16 19:03:04  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
 * Revision 1.32  2000/03/13 00:02:34  steve
 *  Remove unneeded templates.
 *
 * Revision 1.31  2000/02/23 04:43:43  steve
 *  Some compilers do not accept the not symbol.
 *
 * Revision 1.30  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.29  2000/01/08 03:09:14  steve
 *  Non-blocking memory writes.
 */
#endif
