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
#if !defined(WINNT)
#ident "$Id: udp.h,v 1.9 2001/08/09 19:38:23 steve Exp $"
#endif

#include "pointers.h"
#include "functor.h"

typedef struct udp_table_entry_s *udp_table_entry_t;

class vvp_udp_s : public vvp_fobj_s
{
    public:
      void set(vvp_ipoint_t i, functor_t f, bool push);

    public:
      char *name;
      udp_table_entry_t table;
      unsigned short ntable;
      unsigned short sequ;
      unsigned short nin;
      unsigned char  init;

      void compile_table(char **tab);

 private:
      void compile_row_(udp_table_entry_t, char *);
      unsigned char propagate_(vvp_ipoint_t i);
};

struct vvp_udp_s *udp_create(char *label);
struct vvp_udp_s *udp_find(char *label);


/*
 * $Log: udp.h,v $
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
