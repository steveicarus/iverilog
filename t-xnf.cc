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
#ident "$Id: t-xnf.cc,v 1.1 1998/11/16 05:03:53 steve Exp $"
#endif

# include  "netlist.h"
# include  "target.h"

class target_xnf  : public target_t {

    public:
      void start_design(ostream&os, const Design*);
      void end_design(ostream&os, const Design*);
      void logic(ostream&os, const NetLogic*);

    private:
      static string mangle(const string&);
};

/*
 * This function takes a signal name and mangles it into an equivilent
 * name that is suitable to the XNF format.
 */
string target_xnf::mangle(const string&name)
{
      string result;
      for (unsigned idx = 0 ;  idx < name.length() ;  idx += 1)
	    switch (name[idx]) {
		case '.':
		  result = result + "/";
		  break;
		default:
		  result = result + name[idx];
		  break;
	    }

      return result;
}


void target_xnf::start_design(ostream&os, const Design*)
{
      os << "LCANET,6" << endl;
      os << "PROG,verilog,0.0,\"Steve's Verilog\"" << endl;
      os << "PART,4000-10" << endl;
}

void target_xnf::end_design(ostream&os, const Design*)
{
      os << "EOF" << endl;
}

void target_xnf::logic(ostream&os, const NetLogic*net)
{
      os << "SYM," << net->name() << ", ";
      switch (net->type()) {
	  case NetLogic::AND:
	    os << "AND";
	    break;
	  case NetLogic::NAND:
	    os << "NAND";
	    break;
	  case NetLogic::NOR:
	    os << "NOR";
	    break;
	  case NetLogic::OR:
	    os << "OR";
	    break;
	  case NetLogic::XNOR:
	    os << "XNOR";
	    break;
	  case NetLogic::XOR:
	    os << "XOR";
	    break;
      }
      os << endl;

      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    unsigned cpin;
	    const NetObj*cur;
	    for (net->pin(idx).next_link(cur, cpin)
		       ; (cur != net) || (cpin != idx)
		       ; cur->pin(cpin).next_link(cur, cpin)) {

		  const NetNet*sig = dynamic_cast<const NetNet*>(cur);
		  if (sig) {
			os << "PIN,";
			if (idx == 0)
			      os << "O,O,";
			else
			      os << "I" << idx-1 << ",I,";
			os << mangle(sig->name());
			if (sig->pin_count() > 1)
			      os << "<" << cpin << ">";

			os << ",," << endl;
		  }
	    }
      }

      os << "END" << endl;
}

static target_xnf target_xnf_obj;

extern const struct target tgt_xnf = { "xnf", &target_xnf_obj };

/*
 * $Log: t-xnf.cc,v $
 * Revision 1.1  1998/11/16 05:03:53  steve
 *  Add the sigfold function that unlinks excess
 *  signal nodes, and add the XNF target.
 *
 */

