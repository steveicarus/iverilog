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
#ident "$Id: t-xnf.cc,v 1.3 1998/11/23 00:20:24 steve Exp $"
#endif

/* XNF BACKEND
 * This target supports generating Xilinx Netlist Format netlists for
 * use by Xilinx tools, and other tools that accepts Xilinx designs.
 *
 * FLAGS
 * The XNF backend uses the following flags from the command line to
 * affect the generated file:
 *
 *   part=<foo>
 *	Specify the part type. The part string is written into the
 *	PART record. Valid types are defined by Xilinx or the
 *	receiving tools
 */

# include  "netlist.h"
# include  "target.h"

class target_xnf  : public target_t {

    public:
      void start_design(ostream&os, const Design*);
      void end_design(ostream&os, const Design*);
      void signal(ostream&os, const NetNet*);
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


void target_xnf::start_design(ostream&os, const Design*des)
{
      os << "LCANET,6" << endl;
      os << "PROG,verilog,0.0,\"Steve's Verilog\"" << endl;
      os << "PART," << des->get_flag("part") << endl;
}

void target_xnf::end_design(ostream&os, const Design*)
{
      os << "EOF" << endl;
}

void scrape_pad_info(string&str, char&dir, unsigned&num)
{
      while (str[0] == ' ')
	    str = str.substr(1);

      switch (str[0]) {
	  case 'b':
	  case 'B':
	    dir = 'B';
	    break;
	  case 'o':
	  case 'O':
	    dir = 'O';
	    break;
	  case 'i':
	  case 'I':
	    dir = 'I';
	    break;
	  case 't':
	  case 'T':
	    dir = 'T';
	    break;
	  default:
	    dir = '?';
	    break;
      }

      str = str.substr(1);
      unsigned val = 0;
      while (str.size() && isdigit(str[0])) {
	    val = val * 10 + (str[0]-'0');
	    str = str.substr(1);
      }
      num = val;

      while (str.size() && str[0] == ' ')
	    str = str.substr(1);

      if (str.size() && str[0] == ',')
	    str = str.substr(1);

}

/*
 * Look for signals that have attributes that are pertinent to XNF
 * files. The most obvious are those that have the PAD attribute.
 *
 * Individual signals are easy, the pad description is a letter
 * followed by a decimal number that is the pin.
 *
 * The PAD attribute for a vector is a comma separated pin
 * descriptions, that enumerate the pins from most significant to
 * least significant.
 */
void target_xnf::signal(ostream&os, const NetNet*net)
{
      string pad = net->attribute("PAD");
      if (pad != "") {

	    if (net->pin_count() == 1) {
		  char dir;
		  unsigned num;
		  scrape_pad_info(pad, dir, num);
		  os << "EXT, " << mangle(net->name()) << ", " << dir
		     << ", " << num << endl;

	    } else for (unsigned idx = net->pin_count(); idx > 0; idx -= 1) {

		  char dir;
		  unsigned num;
		  scrape_pad_info(pad, dir, num);
		  os << "EXT, " << mangle(net->name()) << "<" << (idx-1)
		     << ">, " << dir << ", " << num << endl;
	    }
      }
}

/*
 * The logic gates I know so far can be translated directly into XNF
 * standard symbol types. This is a fairly obvious transformation.
 */
void target_xnf::logic(ostream&os, const NetLogic*net)
{
      os << "SYM, " << mangle(net->name()) << ", ";
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
	  case NetLogic::NOT:
	    os << "INV";
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
      os << ", LIBVER=2.0.0" << endl;

      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    unsigned cpin;
	    const NetObj*cur;
	    for (net->pin(idx).next_link(cur, cpin)
		       ; (cur != net) || (cpin != idx)
		       ; cur->pin(cpin).next_link(cur, cpin)) {

		  const NetNet*sig = dynamic_cast<const NetNet*>(cur);
		  if (sig) {
			os << "    PIN, ";
			if (idx == 0) {
			      os << "O, O, ";
			} else {
			      os << "I";
			      if (net->pin_count() > 2)
				    os << idx-1;
			      os << ", I, ";
			}
			os << mangle(sig->name());
			if (sig->pin_count() > 1)
			      os << "<" << cpin << ">";

			os << endl;
		  }
	    }
      }

      os << "END" << endl;
}

static target_xnf target_xnf_obj;

extern const struct target tgt_xnf = { "xnf", &target_xnf_obj };

/*
 * $Log: t-xnf.cc,v $
 * Revision 1.3  1998/11/23 00:20:24  steve
 *  NetAssign handles lvalues as pin links
 *  instead of a signal pointer,
 *  Wire attributes added,
 *  Ability to parse UDP descriptions added,
 *  XNF generates EXT records for signals with
 *  the PAD attribute.
 *
 * Revision 1.2  1998/11/18 04:25:22  steve
 *  Add -f flags for generic flag key/values.
 *
 * Revision 1.1  1998/11/16 05:03:53  steve
 *  Add the sigfold function that unlinks excess
 *  signal nodes, and add the XNF target.
 *
 */

