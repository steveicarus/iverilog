/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: udp.cc,v 1.25 2003/09/24 20:46:48 steve Exp $"
#endif

#include "udp.h"
#include "schedule.h"
#include "symbols.h"
#include <assert.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdlib.h>
#include <stdio.h>

/*
 * This method is called when the input of a slice of the UDP
 * changes. All the slices of the UDP point to this common functor,
 * that manages the output of the UDP device. The input functors are
 * all edge_inputs_functors_s objects.
 */
void udp_functor_s::set(vvp_ipoint_t i, bool push, unsigned val, unsigned)
{
	// Save the input in the ival member of this functor. It will
	// be read by the propagate method. The old_ival method of the
	// edge_input_functor (I am that) will be set by propagate.
      put(i, val);
      unsigned char out = udp->propagate(this, i);

	// Send the result to the output. If this is a combinational
	// UDP, then push according to the push flag. However, do
	// *not* push sequential outputs.

	// Sequential primitive outputs are scheduled as active
	// events, no matter what common sense and reason say.
      put_oval(out, push & !udp->sequ, false);
}


static symbol_table_t udp_table;

struct vvp_udp_s *udp_create(char *label)
{
  if (!udp_table)
    udp_table = new_symbol_table();

  assert(!udp_find(label));

  struct vvp_udp_s *u = new vvp_udp_s;
  
  symbol_value_t v;
  v.ptr = u;
  sym_set_value(udp_table, label, v);

  u->name = 0x0;
  u->sequ = 0;
  u->nin = 0;
  u->init = 3;
  u->table = 0x0;

  return u;
}

struct vvp_udp_s *udp_find(char *label)
{
  symbol_value_t v = sym_get_value(udp_table, label);
  return (struct vvp_udp_s *)v.ptr;
}

typedef unsigned int udp_vec_t;
struct udp_table_entry_s 
{
  udp_vec_t not_0;          // all inputs that must not be 0
  udp_vec_t not_1x;         // all inputs that must not be 1, x
  unsigned char edge_idx;   // input index of the edge
  unsigned char edge_type;  // permissible transitions. 0: no edge.
  unsigned char out;        // new output, 0..2
};

enum edge_type_e 
{
  EDGE_0   = 0x01,
  EDGE_1   = 0x02,
  EDGE_x   = 0x0c,
  EDGE_any = 0x0f,
};

/*
 * This method of the common table object for the UDP calculates the
 * output based on the new input of the functor calling me.
 */
unsigned char vvp_udp_s::propagate(functor_t fu, vvp_ipoint_t uix)
{
  vvp_ipoint_t base = ipoint_make(uix, 0);

  unsigned char ret = 2;

  unsigned edge_idx = 0;  // input index that changed
  unsigned edge_type = 0; // input transition

  udp_vec_t invec = 0x0; // vector of 2-bit inputs

  for (unsigned i=0; i < nin;  i+=4)
    {
      int idx = ipoint_input_index(base, i);
      edge_inputs_functor_s *pfun = 
	    dynamic_cast<edge_inputs_functor_s *>(functor_index(idx));
      assert(pfun);

      invec |= pfun->ival << (2*i);

      unsigned char diff = pfun->ival ^ pfun->old_ival;
      if (diff)
	{
	  unsigned ii = 0;
	  if (diff & 0x03) ii = 0;
	  if (diff & 0x0c) ii = 1;
	  if (diff & 0x30) ii = 2;
	  if (diff & 0xc0) ii = 3;

	  edge_idx = i+ii;

	  unsigned old_in = (pfun->old_ival >> (2*ii)) & 3;
	  edge_type = (1<<old_in);
	}

      pfun->old_ival = pfun->ival;
    }

  if (sequ)
    {
      if (edge_type == 0)
           return fu->get_oval();
      invec <<= 2;
      invec |= (fu->get_oval() & 3);
    }

  udp_vec_t inx  =  invec & 0xaaaaaaaaU; // all 'x'/'z'
  udp_vec_t in01 =  ~(inx>>1);           // masks all 'x'/'z'
  udp_vec_t in1x =  invec & in01;        // all 'x' and '1'
  udp_vec_t in0  = ~invec & in01;        // all '0'

  for (unsigned ri=0; ri < ntable;  ri++)
    {
      udp_table_entry_t row = table+ri;

      if ((in1x & row->not_1x) || (in0 & row->not_0))
	continue;

      if (!row->edge_type)
	{
	  ret = row->out;
	  break;
	}
      
      if (row->edge_idx != edge_idx)
	continue;

      if (row->edge_type & edge_type)
	{
	  ret = row->out;
	  break;
	}
    }

  if (ret>2)
    ret = fu->get_oval();

  return ret;
}

void vvp_udp_s::compile_table(char **tab)
{
  ntable = 0;
  for (char **ss = tab; *ss; ss++)
    ntable++;
  table = new struct udp_table_entry_s[ntable];
  for (unsigned i = 0; i < ntable; i++)
    {
      compile_row_(&table[i], tab[i]);
      free(tab[i]);
    }
  free(tab);
}

void vvp_udp_s::compile_row_(udp_table_entry_t row, char *rchr)
{
  row->not_0  = 0;     // all inputs that must not be 0
  row->not_1x = 0;     // all inputs that must not be 1 or x
  row->edge_idx = 0;   // input index of the edge
  row->edge_type = 0;  // permissible transitions. 0: no edge.

  char *s = rchr;
  for (unsigned i = (sequ ? 0 : 1); i <= nin; i++)
    {
      char c = *s;
      s++;

      unsigned char n0 = 0;
      unsigned char n1x = 0;
      unsigned char edge = 0;

      switch (c)
	{
	default:
	  fprintf(stderr, "vvp: Illegal character (%d) in UDP table\n", c);
	  assert(0);
	  break;

	case '?':
	  break;
	case '0':
	  n1x = 3; // 1, x not allowed
	  break;
	case '1':
	  n0  = 1; // 0 not allowed
	  n1x = 2; // x not allowed
	  break;
	case 'x':
	  n0  = 1; // 0 not allowed
	  n1x = 1; // 1 not allowed
	  break;
	case 'b':
	  n1x = 2; // x not allowed
	  break;
	case 'l':
	  n1x = 1; // 1 not allowed
	  break;
	case 'h':
	  n0 = 1;  // 0 not allowed
	  break;

	case '*':
	  edge = EDGE_any;
	  break;

	case '+':
	  n0  = 1; // 0 not allowed
	  n1x = 2; // x not allowed
	  edge = EDGE_any;
	  break;
	case '_':
	  n1x = 3; // 1, x not allowed
	  edge = EDGE_any;
	  break;
	case '%':
	  n0  = 1; // 0 not allowed
	  n1x = 1; // 1 not allowed
	  edge = EDGE_any;
	  break;
	  
	case 'N':
	  edge = EDGE_1;
	  break;
	case 'P':
	  edge = EDGE_0;
	  break;
	case 'B':
	  edge = EDGE_x;
	  break;

	case 'r':
	  n0  = 1; // 0 not allowed
	  n1x = 2; // x not allowed
	  edge = EDGE_0;
	  break;
	case 'R':
	  n0  = 1; // 0 not allowed
	  n1x = 2; // x not allowed
	  edge = EDGE_x;
	  break;
	case 'f':
	  n1x = 3; // 1, x not allowed
	  edge = EDGE_1;
	  break;
	case 'F':
	  n1x = 3; // 1, x not allowed
	  edge = EDGE_x;
	  break;
	case 'Q':
	  n0  = 1; // 0 not allowed
	  n1x = 1; // 1 not allowed
	  edge = EDGE_0;
	  break;
	case 'q':
	  n0  = 1; // 0 not allowed
	  n1x = 1; // 1 not allowed
	  edge = EDGE_0 | EDGE_1;
	  break;
	case 'M':
	  n0  = 1; // 0 not allowed
	  n1x = 1; // 1 not allowed
	  edge = EDGE_1;
	  break;

	case 'n':
	  n1x = 1; // 1 not allowed
	  edge = EDGE_1 | EDGE_x;
	  break;
	case 'p':
	  n0 = 1; // 0 not allowed
	  edge = EDGE_0 | EDGE_x;
	  break;
	case 'v':
	  n1x = 2; // x not allowed
	  edge = EDGE_0 | EDGE_1;
	  break;
	}
      
      if (edge)
	{
	  if (!sequ)
	    {
	      fprintf(stderr, "vvp: edge in combinatorial UDP\n");
	      assert(0);
	    }
	  if (!i)
	    {
	      fprintf(stderr, "vvp: edge in UDP output state\n");
	      assert(0);
	    }
	  row->edge_idx = i-1;
	  if (row->edge_type)
	    {
	      fprintf(stderr, "vvp: multiple edges in UDP table row\n");
	      assert(0);
	    }
	  row->edge_type = edge;
	}

      int j = sequ ? i : i-1;
      row->not_0  |= n0  << (2*j);
      row->not_1x |= n1x << (2*j);
    }

  switch (*s)
    {
    case '0':
      row->out = 0;
      break;
    case '1':
      row->out = 1;
      break;
    case 'x':
      row->out = 2;
      break;
    case '-':
      row->out = 4;
      break;
    default:
      fprintf(stderr, "vvp: illegal character (%d) in udp output spec\n", *s);
      assert(0);
    }
  
}

/*
 * $Log: udp.cc,v $
 * Revision 1.25  2003/09/24 20:46:48  steve
 *  Standard udp scheduling behavior.
 *
 * Revision 1.24  2003/09/17 03:39:55  steve
 *  Internal documentation of UDP devices.
 *
 * Revision 1.23  2003/09/13 00:59:02  steve
 *  Comments.
 *
 * Revision 1.22  2003/09/09 00:56:45  steve
 *  Reimpelement scheduler to divide nonblocking assign queue out.
 *
 * Revision 1.21  2003/06/17 21:28:59  steve
 *  Remove short int restrictions from vvp opcodes. (part 2)
 *
 * Revision 1.20  2003/04/01 05:32:56  steve
 *  Propagate output of sequential udp like non-blocksing assign.
 *
 * Revision 1.19  2003/03/18 01:32:33  steve
 *  Add the q edge flag.
 *
 * Revision 1.18  2003/02/09 23:33:26  steve
 *  Spelling fixes.
 *
 * Revision 1.17  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.16  2002/01/20 23:27:51  steve
 *  return calculated oval from UDP (Stephan Boettcher)
 *
 * Revision 1.15  2002/01/06 17:35:01  steve
 *  Feedback output, not propagated output. (Stephan Boettcher)
 *
 * Revision 1.14  2001/12/06 03:31:25  steve
 *  Support functor delays for gates and UDP devices.
 *  (Stephan Boettcher)
 *
 * Revision 1.13  2001/11/07 03:34:42  steve
 *  Use functor pointers where vvp_ipoint_t is unneeded.
 *
 * Revision 1.12  2001/10/31 04:27:47  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.11  2001/09/19 04:10:40  steve
 *  Change UDP output only if table matches.
 *
 * Revision 1.10  2001/09/15 18:27:05  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.9  2001/08/09 19:38:23  steve
 *  Nets (wires) do not use their own functors.
 *  Modifications to propagation of values.
 *  (Stephan Boettcher)
 *
 * Revision 1.8  2001/07/24 01:44:50  steve
 *  Fast UDP tables (Stephan Boettcher)
 *
 * Revision 1.7  2001/07/16 19:08:32  steve
 *  Schedule instead of propagating UDP output. (Stephan Boettcher)
 *
 * Revision 1.6  2001/06/18 00:51:23  steve
 *  Add more UDP edge types, and finish up compile
 *  and run-time support. (Stephan Boettcher)
 *
 * Revision 1.5  2001/05/31 04:12:43  steve
 *  Make the bufif0 and bufif1 gates strength aware,
 *  and accurately propagate strengths of outputs.
 *
 * Revision 1.4  2001/05/06 03:51:37  steve
 *  Regularize the mode-42 functor handling.
 *
 * Revision 1.3  2001/04/26 15:52:22  steve
 *  Add the mode-42 functor concept to UDPs.
 *
 * Revision 1.2  2001/04/26 03:10:55  steve
 *  Redo and simplify UDP behavior.
 *
 * Revision 1.1  2001/04/24 02:23:59  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 */
