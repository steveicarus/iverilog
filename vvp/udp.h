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
#ident "$Id: udp.h,v 1.1 2001/04/24 02:23:59 steve Exp $"
#endif

#include "pointers.h"
#include "functor.h"
#include "udp.h"

struct vvp_udp_s 
{
  char *name;
  unsigned short sequ;
  unsigned short nin;
  unsigned char  init;
  char **table;
};

void udp_init_links(vvp_ipoint_t fdx, struct vvp_udp_s *u);
struct vvp_udp_s *udp_create(char *label);
struct vvp_udp_s *udp_find(char *label);
unsigned char udp_propagate(vvp_ipoint_t);

// The iterator through the input port list/tree/whatever

class udp_idx_t
{
public:
  udp_idx_t(vvp_ipoint_t, struct vvp_udp_s *);
  vvp_ipoint_t ipoint();
  functor_t functor();
  bool next();
  vvp_ipoint_t next_node();
  bool done();
  vvp_ipoint_t parent();
  void reset();
private:
  vvp_ipoint_t root;
  unsigned nin;
  unsigned cur_i;
};

inline void udp_idx_t::reset()
{
  cur_i = 0;
}

inline udp_idx_t::udp_idx_t(vvp_ipoint_t idx, struct vvp_udp_s *u)
  : root(idx), nin(u->nin)
{
  reset();
}

inline bool udp_idx_t::done()
{
  return cur_i >= nin;
}

inline bool udp_idx_t::next()
{
  cur_i++;
  return !done();
}

inline vvp_ipoint_t udp_idx_t::ipoint()
{
  // The last node in the chain can hold 4 inputs.
  int idx = (cur_i==nin-1 && cur_i ? cur_i-1 : cur_i) / 3;
  int port = cur_i - 3*idx;
  return ipoint_make(ipoint_index(root, idx), port);
}

inline functor_t udp_idx_t::functor()
{
  return functor_index(ipoint());
}

// junp to the next node, and return the ipoint.

inline vvp_ipoint_t udp_idx_t::next_node()
{
  // We omit the last-node case, since is is a don't care here.
  int idx = cur_i / 3 + 1;
  cur_i = idx*3+1;
  return cur_i < nin 
    ? ipoint_make(ipoint_index(root, idx),0)
    : 0;
}

inline vvp_ipoint_t udp_idx_t::parent()
{
  int idx = (cur_i==nin-1 ? cur_i-1 : cur_i) / 3;
  if (!idx)
    return 0x0;
  return ipoint_make(ipoint_index(root, idx-1), 3);
}

/*
 * $Log: udp.h,v $
 * Revision 1.1  2001/04/24 02:23:59  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 */
#endif
