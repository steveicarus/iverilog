#ifndef __udp_H
#define __udp_H
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: udp.h,v 1.12 2003/06/17 21:28:59 steve Exp $"
#endif

#include "functor.h"

// UDP implementation:

typedef struct udp_table_entry_s *udp_table_entry_t;

struct vvp_udp_s
{
      char *name;
      udp_table_entry_t table;
      unsigned ntable;
      unsigned sequ;
      unsigned nin;
      unsigned char init;
  
      void compile_table(char **tab);
      unsigned char propagate(functor_t fu, vvp_ipoint_t i);
  
    private:
      void compile_row_(udp_table_entry_t, char *);
};

struct vvp_udp_s *udp_create(char *label);
struct vvp_udp_s *udp_find(char *label);


// UDP instances:

class udp_functor_s : public edge_inputs_functor_s
{
public:
      explicit udp_functor_s(vvp_udp_s *u) : udp(u) {}
      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
      vvp_udp_s *udp;
};


/*
 * $Log: udp.h,v $
 * Revision 1.12  2003/06/17 21:28:59  steve
 *  Remove short int restrictions from vvp opcodes. (part 2)
 *
 * Revision 1.11  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.10  2001/10/31 04:27:47  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.9  2001/08/09 19:38:23  steve
 *  Nets (wires) do not use their own functors.
 *  Modifications to propagation of values.
 *  (Stephan Boettcher)
 *
 * Revision 1.8  2001/07/24 01:44:50  steve
 *  Fast UDP tables (Stephan Boettcher)
 *
 * Revision 1.6  2001/05/06 03:51:37  steve
 *  Regularize the mode-42 functor handling.
 *
 * Revision 1.5  2001/04/28 20:09:05  steve
 *  Excessive header include.
 *
 * Revision 1.4  2001/04/26 15:52:22  steve
 *  Add the mode-42 functor concept to UDPs.
 *
 * Revision 1.3  2001/04/26 03:10:55  steve
 *  Redo and simplify UDP behavior.
 *
 * Revision 1.2  2001/04/24 03:48:53  steve
 *  Fix underflow when UDP has 1 input.
 *
 * Revision 1.1  2001/04/24 02:23:59  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 */
#endif
