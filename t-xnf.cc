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
#ident "$Id: t-xnf.cc,v 1.7 1999/07/17 03:39:11 steve Exp $"
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
 *	receiving tools.
 *
 * WIRE ATTRIBUTES
 *
 *   PAD = <io><n>
 *      Tell the XNF generator that this wire goes to a PAD. The <io>
 *      is a single character that tells the direction, and <n> is the
 *      pin number. For example, "o31" is output on pin 31. The PAD
 *      attribute is not practically connected to a vector, as all the
 *      bits would go to the same pad.
 *
 * NODE ATTRIBUTES
 *
 *   XNF-LCA = <lname>:<pin>,<pin>...
 *      Specify the LCA library part type for the gate. The lname
 *      is the name of the symbol to use (i.e. DFF) and the comma
 *      separated list is the names of the pins, in the order they
 *      appear in the verilog source. If the name is prefixed with a
 *      tilde (~) then the pin is inverted, and the proper "INV" token
 *      will be added to the PIN record.
 *
 *      This attribute can override even the typical generation of
 *      gates that one might naturally expect of the code generator,
 *      but may be used by the optimizers for placing parts.
 *
 *      An example is "XNF-LCA=OBUF:O,~I". This attribute means that
 *      the object is an OBUF. Pin 0 is called "O", and pin 1 is
 *      called "I". In addition, pin 1 is inverted.
 */

# include  "netlist.h"
# include  "target.h"

class target_xnf  : public target_t {

    public:
      void start_design(ostream&os, const Design*);
      void end_design(ostream&os, const Design*);
      void signal(ostream&os, const NetNet*);
      void logic(ostream&os, const NetLogic*);
      void udp(ostream&os,  const NetUDP*);

    private:
      static string mangle(const string&);
      static void draw_pin(ostream&os, const string&name,
			   const NetObj::Link&lnk);
      static void draw_sym_with_lcaname(ostream&os, string lca,
					const NetNode*net);
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

void target_xnf::draw_pin(ostream&os, const string&name,
			  const NetObj::Link&lnk)
{
      bool inv = false;
      string use_name = name;
      if (use_name[0] == '~') {
	    inv = true;
	    use_name = use_name.substr(1);
      }

      char type;
      switch (lnk.get_dir()) {
	  case NetObj::Link::INPUT:
	  case NetObj::Link::PASSIVE:
	    type = 'I';
	    break;
	  case NetObj::Link::OUTPUT:
	    type = 'O';
	    break;
      }
	    
      unsigned cpin;
      const NetObj*cur;
      for (lnk.next_link(cur, cpin)
		 ; cur->pin(cpin) != lnk
		 ; cur->pin(cpin).next_link(cur, cpin)) {

	    const NetNet*sig = dynamic_cast<const NetNet*>(cur);
	    if (sig) {
		  os << "    PIN, " << use_name << ", " << type << ", "
		     << mangle(sig->name());
		  if (sig->pin_count() > 1)
			os << "<" << cpin << ">";

		  if (inv)
			os << ",,INV";

		  os << endl;
	    }
      }
}

static string scrape_pin_name(string&list)
{
      unsigned idx = list.find(',');
      string name = list.substr(0, idx);
      list = list.substr(idx+1);
      return name;
}

/*
 * This method draws an LCA item based on the XNF-LCA attribute
 * given. The LCA attribute gives enough information to completely
 * draw the node in XNF, which is pretty handy at this point.
 */
void target_xnf::draw_sym_with_lcaname(ostream&os, string lca,
				       const NetNode*net)
{
      unsigned idx = lca.find(':');
      string lcaname = lca.substr(0, idx);
      lca = lca.substr(idx+1);

      os << "SYM, " << mangle(net->name()) << ", " << lcaname
	 << ", LIBVER=2.0.0" << endl;

      for (idx = 0 ;  idx < net->pin_count() ;  idx += 1)
	    draw_pin(os, scrape_pin_name(lca), net->pin(idx));

      os << "END" << endl;
}

void target_xnf::start_design(ostream&os, const Design*des)
{
      os << "LCANET,6" << endl;
      os << "PROG,verilog,0.0,\"Icarus Verilog\"" << endl;
      os << "PART," << des->get_flag("part") << endl;
}

void target_xnf::end_design(ostream&os, const Design*)
{
      os << "EOF" << endl;
}

void scrape_pad_info(string str, char&dir, unsigned&num)
{
	// Get rid of leading white space
      while (str[0] == ' ')
	    str = str.substr(1);

	// Get the direction letter
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

	// Get the number part.
      str = str.substr(1);
      unsigned val = 0;
      while (str.size() && isdigit(str[0])) {
	    val = val * 10 + (str[0]-'0');
	    str = str.substr(1);
      }
      num = val;
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
      if (pad == "")
	    return;

      if (net->pin_count() > 1) {
	    cerr << "Signal ``" << net->name() << "'' with PAD=" <<
		  pad << " is a vector." << endl;
	    return;
      }

      char dir;
      unsigned num;
      scrape_pad_info(pad, dir, num);
      os << "EXT, " << mangle(net->name()) << ", " << dir
	 << ", " << num << endl;
}

/*
 * The logic gates I know so far can be translated directly into XNF
 * standard symbol types. This is a fairly obvious transformation.
 */
void target_xnf::logic(ostream&os, const NetLogic*net)
{
	// The XNF-LCA attribute overrides anything I might guess
	// about this object.
      string lca = net->attribute("XNF-LCA");
      if (lca != "") {
	    draw_sym_with_lcaname(os, lca, net);
	    return;
      }

      os << "SYM, " << mangle(net->name()) << ", ";
      switch (net->type()) {
	  case NetLogic::AND:
	    os << "AND";
	    break;
	  case NetLogic::BUF:
	    os << "BUF";
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
	  default:
	    cerr << "XNF: Unhandled logic type." << endl;
	    break;
      }
      os << ", LIBVER=2.0.0" << endl;

      draw_pin(os, "O", net->pin(0));

      if (net->pin_count() == 2) {
	    draw_pin(os, "I", net->pin(1));
      } else for (unsigned idx = 1 ;  idx < net->pin_count() ;  idx += 1) {
	    string name = "I";
	    assert(net->pin_count() <= 11);
	    name += (char)('0'+idx-1);
	    draw_pin(os, name, net->pin(idx));
      }

      os << "END" << endl;
}

void target_xnf::udp(ostream&os, const NetUDP*net)
{
      string lca = net->attribute("XNF-LCA");

	// I only know how to draw a UDP if it has the XNF-LCA
	// attribute attached to it.
      if (lca == "") {
	    cerr << "I don't understand this UDP." << endl;
	    return;
      }

      draw_sym_with_lcaname(os, lca, net);
}

static target_xnf target_xnf_obj;

extern const struct target tgt_xnf = { "xnf", &target_xnf_obj };

/*
 * $Log: t-xnf.cc,v $
 * Revision 1.7  1999/07/17 03:39:11  steve
 *  simplified process scan for targets.
 *
 * Revision 1.6  1998/12/09 02:43:19  steve
 *  Fix 2pin logic gates.
 *
 * Revision 1.5  1998/12/07 04:53:17  steve
 *  Generate OBUF or IBUF attributes (and the gates
 *  to garry them) where a wire is a pad. This involved
 *  figuring out enough of the netlist to know when such
 *  was needed, and to generate new gates and signales
 *  to handle what's missing.
 *
 * Revision 1.4  1998/12/02 04:37:13  steve
 *  Add the nobufz function to eliminate bufz objects,
 *  Object links are marked with direction,
 *  constant propagation is more careful will wide links,
 *  Signal folding is aware of attributes, and
 *  the XNF target can dump UDP objects based on LCA
 *  attributes.
 *
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

