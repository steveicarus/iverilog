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
#ident "$Id: udp.cc,v 1.1 2001/04/24 02:23:59 steve Exp $"
#endif

#include "udp.h"
#include "symbols.h"
#include <assert.h>
#include <malloc.h>

static symbol_table_t udp_table;

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

void udp_init_links(vvp_ipoint_t fdx, struct vvp_udp_s *u)
{
  udp_idx_t ux(fdx, u);
  do
    {
      vvp_ipoint_t pa = ux.parent();
      if (pa)
	{
	  functor_t fu = ux.functor();
	  functor_t fp = functor_index(pa);
	  fp->port[ipoint_port(pa)] = 0x0;
	  fu->out = pa;
	  fu->udp = 0x0;
	  fu->mode = 3;
	  
	  fu->ival = 0xaa;
	  fu->old_ival = fu->ival;
	}
    } while (ux.next_node());
}

unsigned char udp_propagate(vvp_ipoint_t uix)
{
  functor_t fu = functor_index(uix);
  struct vvp_udp_s *u = fu->udp;
  assert(u);
  assert(u->table);

  udp_idx_t ux(uix, u);

  unsigned char ret = 2;

  for (char **rptr = u->table; *rptr ; rptr++) 
    {
      char *row = *rptr;

      if (u->sequ)
	{
	  if (row[0]=='?' || row[0]==(fu->oval&3)["01xx"])
	    row++;
	  else
	    continue;
	}

      ux.reset();

      do
	{
	  assert (*row);
	  
	  int port = ipoint_port(ux.ipoint());
	  functor_t pfun = ux.functor();
	  
	  char new_bit = ((pfun->ival >> (2*port))&3)["01xx"];

	  if (*row != new_bit  &&  *row != '?')
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
	} while (row++, ux.next());
      
      if (ux.done())
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
 
  ux.reset();
  do
    {
      functor_t fu = ux.functor();
      fu->old_ival = fu->ival;
    } while (ux.next_node());
  
  return ret;
}

/*
 * $Log: udp.cc,v $
 * Revision 1.1  2001/04/24 02:23:59  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 */
