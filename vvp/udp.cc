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
#if !defined(WINNT)
#ident "$Id: udp.cc,v 1.4 2001/05/06 03:51:37 steve Exp $"
#endif

#include "udp.h"
#include "symbols.h"
#include <assert.h>
#include <malloc.h>

static symbol_table_t udp_table;


void vvp_udp_s::set(vvp_ipoint_t ptr, functor_t fp, bool)
{
  unsigned char out = propagate_(ptr);

  if (out != fp->oval) 
    {
      fp->oval = out;
      functor_propagate(ptr);
    }
}

unsigned vvp_udp_s::get(vvp_ipoint_t i, functor_t f)
{
      assert(0);
}

struct vvp_udp_s *udp_create(char *label)
{
  if (!udp_table)
    udp_table = new_symbol_table();

  assert(!udp_find(label));

  struct vvp_udp_s *u = new struct vvp_udp_s;
  
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

unsigned char vvp_udp_s::propagate_(vvp_ipoint_t uix)
{
  functor_t fu = functor_index(uix);

  unsigned char ret = 2;

  for (char **rptr = table; *rptr ; rptr++) 
    {
      char *row = *rptr;

      if (sequ)
	{
	  char old_out = (fu->oval&3)["01xx"];
	  if (  row[0]=='?' 
	    ||  row[0]==old_out
	    || (row[0]=='b' && old_out!='x')
	    || (row[0]=='l' && old_out!='1')
	    || (row[0]=='h' && old_out!='0') )
	    row++;
	  else
	    continue;
	}

      int i;

      for (i=0;  i < nin;  i++, row++)
	{
	  assert (*row);
	  
	  int idx = ipoint_input_index(uix, i);
	  int port = ipoint_port(idx);
	  functor_t pfun = functor_index(idx);
	  
	  char new_bit = ((pfun->ival >> (2*port))&3)["01xx"];

	  if (    *row != new_bit  
	      &&  *row != '?'
	      && (*row != 'b' || new_bit == 'x')
	      && (*row != 'l' || new_bit == '1')
	      && (*row != 'h' || new_bit == '0') )
	    {
	      char old_bit = ((pfun->old_ival >> (2*port))&3)["01xx"];
	      if (new_bit == old_bit)
		break;

	      switch (*row)
		{
		case '*':
		  continue;
		case '_':
		  if (new_bit == '0')
		    continue;
		  break;
		case '+':
		  if (new_bit == '1')
		    continue;
		  break;
		case '%':
		  if (new_bit == 'x')
		    continue;
		  break;
		case 'B':
		  if (old_bit == 'x')
		    continue;
		  break;
		case 'r':
		  if (old_bit=='0' && new_bit=='1')
		    continue;
		  break;
		case 'R':
		  if (old_bit=='x' && new_bit=='1')
		    continue;
		  break;
		case 'f':
		  if (old_bit=='1' && new_bit=='0')
		    continue;
		  break;
		case 'F':
		  if (old_bit=='x' && new_bit=='0')
		    continue;
		  break;
		case 'p':
		  if (old_bit=='0')
		    continue;
		  break;
		case 'n':
		  if (old_bit=='1')
		    continue;
		  break;
		case 'P':
		  if (old_bit=='0' && new_bit=='x')
		    continue;
		  break;
		case 'N':
		  if (old_bit=='1' && new_bit=='x')
		    continue;
		  break;
		}
	      break;
	    }
	}
      
      if (i == nin)
	{
	  assert(*row);
	  if (*row == '-')
	    ret = fu->oval;
	  else 
	    switch (*row) 
	      {
	      case '0':
		ret = 0;
		break;
	      case '1':
		ret = 1;
		break;
	      default:
		ret = 2;
		break;
	      }
	  break;
	}
    }
 
  for (int i=0;  i < nin;  i+=4)
    {
      functor_t fu = functor_index(ipoint_input_index(uix, i));
      fu->old_ival = fu->ival;
    }
  
  return ret;
}

/*
 * $Log: udp.cc,v $
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
