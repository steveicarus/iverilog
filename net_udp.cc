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
#ident "$Id: net_udp.cc,v 1.2 2000/11/04 06:36:24 steve Exp $"
#endif

# include  "netlist.h"

bool NetUDP::set_table(const string&input, char output)
{
      assert((output == '0') || (output == '1') 
	     || (is_sequential() && (output == '-')));

      cm_[input] = output;

      if (is_sequential()) {
	    assert(input.length() == pin_count());
	      /* XXXX Need to check to make sure that the input vector
		 contains a legal combination of characters. */
	    return sequ_glob_(input, output);

      } else {
	    assert(input.length() == (pin_count()-1));
	      /* XXXX Need to check to make sure that the input vector
		 contains a legal combination of characters. In
		 combinational UDPs, only 0, 1, x, and ? are allowed. */

	    return true;
      }
}

void NetUDP::cleanup_table()
{
      for (FSM_::iterator idx = fsm_.begin() ;  idx != fsm_.end() ; idx++) {
	    string str = (*idx).first;
	    state_t_*st = (*idx).second;
	    assert(str[0] == st->out);

	    for (unsigned pin = 0 ;  pin < pin_count() ;  pin += 1) {
		  if (st->pins[pin].zer && st->pins[pin].zer->out == 'x')
			st->pins[pin].zer = 0;
		  if (st->pins[pin].one && st->pins[pin].one->out == 'x')
			st->pins[pin].one = 0;
		  if (st->pins[pin].xxx && st->pins[pin].xxx->out == 'x')
			st->pins[pin].xxx = 0;
	    }
      }

      for (FSM_::iterator idx = fsm_.begin() ;  idx != fsm_.end() ; ) {
	    FSM_::iterator cur = idx;
	    idx ++;

	    state_t_*st = (*cur).second;

	    if (st->out != 'x')
		  continue;

	    for (unsigned pin = 0 ;  pin < pin_count() ;  pin += 1) {
		  if (st->pins[pin].zer)
			goto break_label;
		  if (st->pins[pin].one)
			goto break_label;
		  if (st->pins[pin].xxx)
			goto break_label;
	    }

		    //delete st;
	    fsm_.erase(cur);

      break_label:;
      }
}

char NetUDP::table_lookup(const string&from, char to, unsigned pin) const
{
      assert(pin <= pin_count());
      assert(from.length() == pin_count());
      FSM_::const_iterator idx = fsm_.find(from);
      if (idx == fsm_.end())
	    return 'x';

      state_t_*next;
      switch (to) {
	  case '0':
	    next = (*idx).second->pins[pin].zer;
	    break;
	  case '1':
	    next = (*idx).second->pins[pin].one;
	    break;
	  case 'x':
	    next = (*idx).second->pins[pin].xxx;
	    break;
	  default:
	    assert(0);
	    next = 0;
      }

      return next? next->out : 'x';
}

void NetUDP::set_initial(char val)
{
      assert(is_sequential());
      assert((val == '0') || (val == '1') || (val == 'x'));
      init_ = val;
}

NetUDP::state_t_* NetUDP::find_state_(const string&str)
{
      map<string,state_t_*>::iterator cur = fsm_.find(str);
      if (cur != fsm_.end())
	    return (*cur).second;

      state_t_*st = fsm_[str];
      if (st == 0) {
	    st = new state_t_(pin_count());
	    st->out = str[0];
	    fsm_[str] = st;
      }

      return st;
}

NetUDP_COMB::NetUDP_COMB(const string&n, unsigned pins, bool sequ)
  : NetNode(n, pins), sequential_(sequ)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name("O", 0);
      for (unsigned idx = 1 ;  idx < pins ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name("I", idx-1);
      }
}

bool NetUDP_COMB::set_table(const string&input, char output)
{
      assert((output == '0') || (output == '1'));

      assert(input.length() == (pin_count()-1));
	/* XXXX Need to check to make sure that the input vector
	   contains a legal combination of characters. In
	   combinational UDPs, only 0, 1, x, and ? are allowed. */
      cm_[input] = output;

      return true;
}

void NetUDP_COMB::cleanup_table()
{
}

bool NetUDP_COMB::first(string&inp, char&out) const
{
      idx_ = cm_.begin();
      if (idx_ == cm_.end())
	    return false;

      inp = (*idx_).first;
      out = (*idx_).second;

      return true;
}

bool NetUDP_COMB::next(string&inp, char&out) const
{
      idx_ ++;
      if (idx_ == cm_.end())
	    return false;

      inp = (*idx_).first;
      out = (*idx_).second;

      return true;
}

/*
 * $Log: net_udp.cc,v $
 * Revision 1.2  2000/11/04 06:36:24  steve
 *  Apply sequential UDP rework from Stephan Boettcher  (PR#39)
 *
 * Revision 1.1  2000/03/29 04:37:11  steve
 *  New and improved combinational primitives.
 *
 */

