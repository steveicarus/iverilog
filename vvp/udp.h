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
#ident "$Id: udp.h,v 1.4 2001/04/26 15:52:22 steve Exp $"
#endif

#include "pointers.h"
#include "functor.h"
#include "udp.h"

struct vvp_udp_s : public vvp_fobj_s
{
  char *name;
  unsigned short sequ;
  unsigned short nin;
  unsigned char  init;
  char **table;
};

struct vvp_udp_s *udp_create(char *label);
struct vvp_udp_s *udp_find(char *label);
unsigned char udp_propagate(vvp_ipoint_t);

/*
 * $Log: udp.h,v $
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
