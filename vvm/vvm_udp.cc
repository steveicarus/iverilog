/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvm_udp.cc,v 1.4 2001/06/18 00:51:23 steve Exp $"
#endif

# include  "vvm_gates.h"

#ifdef UDP_DEBUG
#include <iostream>
#endif

vvm_udp_comb::vvm_udp_comb(unsigned w, const char*t)
: vvm_1bit_out(0), width_(w), table_(t)
{
      ibits_ = new char[width_];
      obit_ = 'x';
}

vvm_udp_comb::~vvm_udp_comb()
{
      delete[]ibits_;
}

void vvm_udp_sequ1::init_I(unsigned idx, vpip_bit_t val)
{
  vvm_udp_comb::init_I(idx, val);
}

void vvm_udp_comb::init_I(unsigned idx, vpip_bit_t val)
{
      assert(idx < width_);
      if (B_IS1(val))
	    ibits_[idx] = '1';
      else if (B_IS0(val))
	    ibits_[idx] = '0';
      else
	    ibits_[idx] = 'x';
}

void vvm_udp_sequ1::take_value(unsigned key, vpip_bit_t val)
{
  assert(key < width_ - 1);
  ibits_[0] = obit_;
  vvm_udp_comb::take_value(key+1, val);
#ifdef UDP_DEBUG
  static int max_deb = UDP_DEBUG;
  if (max_deb>0)
    if (--max_deb<1000)
      cerr<<"sUDP(\""<<table_<<"\", \""<<ibits_<<"\")=\""<<obit_<<"\""<<endl;
#endif
}

void vvm_udp_comb::take_value(unsigned key, vpip_bit_t val)
{
      char old_bit = ibits_[key];
      init_I(key, val);

      if (ibits_[key] == old_bit)
	    return;

      for (const char*row = table_ ;  row[0] ;  row += width_+1) {

	    unsigned idx;
	    for (idx = 0 ;  idx < width_ ;  idx += 1)
	      {
		char new_bit = ibits_[idx];
		if (row[idx] != ibits_[idx]
		    && row[idx] != '?'
		    && (row[idx] != 'b' || new_bit == 'x')
		    && (row[idx] != 'l' || new_bit == '1')
		    && (row[idx] != 'h' || new_bit == '0') )
		  {
		    if (idx == key)
		      {
			switch (row[idx])
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
			  case 'P':
			    if (old_bit=='0')
			      continue;
			    break;
			  case 'p':
			    if (old_bit=='0' || new_bit=='1')
			      continue;
			    break;
			  case 'N':
			    if (old_bit=='1')
			      continue;
			    break;
			  case 'n':
			    if (old_bit=='1' || new_bit=='0')
			      continue;
			    break;
			  case 'Q':
			    if (old_bit=='0' && new_bit=='x')
			      continue;
			    break;
			  case 'M':
			    if (old_bit=='1' && new_bit=='x')
			      continue;
			    break;
			  }
			// bad edge
		      }
		    break; // bad edge/level
		  }
	      }

	    if (idx == width_) 
	      {
		if (row[width_] == '-'
		    || row[width_] == obit_)
		  return;

		obit_ = row[width_];
		
		switch (obit_) 
		  {
		  case '0':
		    output(St0);
		    return;
		  case '1':
		    output(St1);
		    return;
		  case 'z':
		    output(HiZ);
		    return;
		  default:
		    output(StX);
		    return;
		  }
	      }
      }

      if (obit_ != 'x')
	{
	  output(StX);
	  obit_ = 'x';
	}
}


/*
 * $Log: vvm_udp.cc,v $
 * Revision 1.4  2001/06/18 00:51:23  steve
 *  Add more UDP edge types, and finish up compile
 *  and run-time support. (Stephan Boettcher)
 *
 * Revision 1.3  2001/04/22 23:09:46  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.2  2000/11/04 06:36:24  steve
 *  Apply sequential UDP rework from Stephan Boettcher  (PR#39)
 *
 * Revision 1.1  2000/03/29 04:37:11  steve
 *  New and improved combinational primitives.
 *
 */

