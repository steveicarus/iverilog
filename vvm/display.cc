/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: display.cc,v 1.2 1998/11/10 00:48:31 steve Exp $"
#endif

# include  "vvm.h"
# include  "vvm_calltf.h"
# include  <iostream>

static void format_bit(ostream&os, class vvm_calltf_parm*parm)
{
      switch (parm->type()) {
	  case vvm_calltf_parm::NONE:
	    os << "z";
	    break;
	  case vvm_calltf_parm::ULONG:
	    os << ((parm->as_ulong()&1) ? "0" : "1");
	    break;
	  case vvm_calltf_parm::STRING:
	    os << parm->as_string();
	    break;
	  case vvm_calltf_parm::BITS:
	    os << parm->as_bits()->get_bit(0);
	    break;
      }
}

static void format_dec(vvm_simulation*sim, ostream&os,
		       class vvm_calltf_parm*parm)
{
      switch (parm->type()) {
	  case vvm_calltf_parm::TIME:
	    os << sim->get_sim_time();
	    break;
	  case vvm_calltf_parm::NONE:
	    os << "0";
	    break;
	  case vvm_calltf_parm::ULONG:
	    os << parm->as_ulong();
	    break;
	  case vvm_calltf_parm::STRING:
	    os << parm->as_string();
	    break;
	  case vvm_calltf_parm::BITS: {
		unsigned long val = 0;
		unsigned long mask = 1;
		const vvm_bits_t*bstr = parm->as_bits();
		for (unsigned idx = 0 ;  idx < bstr->get_width() ;  idx += 1) {
		      if (bstr->get_bit(idx) == V1) val |= mask;
		      mask <<= 1;
		}
		os << val;
		break;
	  }
      }
}

static void format_name(ostream&os, class vvm_calltf_parm*parm)
{
      switch (parm->type()) {
	  case vvm_calltf_parm::TIME:
	    os << "$time";
	    break;
	  case vvm_calltf_parm::NONE:
	    break;
	  case vvm_calltf_parm::ULONG:
	    os << parm->as_ulong();
	    break;
	  case vvm_calltf_parm::STRING:
	    os << "\"" << parm->as_string() << "\"";
	    break;
	  case vvm_calltf_parm::BITS:
	    os << parm->sig_name();
	    break;
      }
}

static unsigned format(vvm_simulation*sim, const string&str,
		       unsigned nparms,
		       class vvm_calltf_parm*parms)
{
      char prev = 0;
      unsigned next_parm = 0;
      unsigned idx = 0;
      while (idx < str.length()) {
	    if (prev == '%') {
		  switch (str[idx]) {
		      case 'b':
		      case 'B':
			format_bit(cout, parms+next_parm);
			next_parm += 1;
			break;
		      case 'd':
		      case 'D':
			format_dec(sim, cout, parms+next_parm);
			next_parm += 1;
			break;
		      case 'm':
		      case 'M':
			format_name(cout, parms+next_parm);
			next_parm += 1;
			break;
		      case '%':
			cout << str[idx];
			break;
		  }		

		  prev = 0;

	    } else {
		  if (str[idx] != '%')
			cout << str[idx];
		  else
			prev = '%';
	    }

	    idx += 1;
      }

      return next_parm;
}

void Sdisplay(vvm_simulation*sim, const string&name,
	      unsigned nparms, class vvm_calltf_parm*parms)
{
      for (unsigned idx = 0 ;  idx < nparms ;  idx += 1)
	    switch (parms[idx].type()) {
		case vvm_calltf_parm::NONE:
		  cout << " ";
		  break;
		case vvm_calltf_parm::TIME:
		  cout << sim->get_sim_time();
		  break;
		case vvm_calltf_parm::ULONG:
		  cout << parms[idx].as_ulong();
		  break;
		case vvm_calltf_parm::STRING:
		  idx += format(sim, parms[idx].as_string(),
				nparms-idx-1, parms+idx+1);
		  break;
		case vvm_calltf_parm::BITS:
		  cout << *parms[idx].as_bits();
		  break;
	    }

      cout << endl;
}

class monitor_event  : public vvm_event {
    public:
      monitor_event(vvm_simulation*sim,
		    unsigned nparms, class vvm_calltf_parm*parms)
	    { sim_ = sim;
	      nparms_ = nparms;
	      parms_ = new vvm_calltf_parm[nparms];
	      for (unsigned idx = 0 ;  idx < nparms_ ;  idx += 1)
		    parms_[idx] = parms[idx];
	    }

      ~monitor_event() { delete[]parms_; }

    private:
      vvm_simulation*sim_;
      unsigned nparms_;
      vvm_calltf_parm*parms_;
      void event_function();
};

void monitor_event::event_function()
{
      Sdisplay(sim_, "$display", nparms_, parms_);
}

static monitor_event*mon = 0;

void Smonitor(vvm_simulation*sim, const string&name,
	      unsigned nparms, class vvm_calltf_parm*parms)
{
      if (mon) delete mon;
      mon = new monitor_event(sim, nparms, parms);

      for (unsigned idx = 0 ;  idx < nparms ;  idx += 1) {
	    if (parms[idx].type() != vvm_calltf_parm::BITS)
		  continue;

	    parms[idx].as_mon()->enable(mon);
      }
}

/*
 * $Log: display.cc,v $
 * Revision 1.2  1998/11/10 00:48:31  steve
 *  Add support it vvm target for level-sensitive
 *  triggers (i.e. the Verilog wait).
 *  Fix display of $time is format strings.
 *
 * Revision 1.1  1998/11/09 23:44:10  steve
 *  Add vvm library.
 *
 */

