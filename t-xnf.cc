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
#ident "$Id: t-xnf.cc,v 1.14 1999/11/05 18:43:12 steve Exp $"
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

      void lpm_add_sub(ostream&os, const NetAddSub*);
      void lpm_ff(ostream&os, const NetFF*);
      void lpm_mux(ostream&os, const NetMux*);

      void net_const(ostream&os, const NetConst*);
      void logic(ostream&os, const NetLogic*);
      void bufz(ostream&os, const NetBUFZ*);
      void udp(ostream&os,  const NetUDP*);

    private:
      static string mangle(const string&);
      static void draw_pin(ostream&os, const string&name,
			   const NetObj::Link&lnk);
      static void draw_sym_with_lcaname(ostream&os, string lca,
					const NetNode*net);
};

/*
 * This function takes a signal name and mangles it into an equivalent
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
      os << "PROG,verilog,0.2PRE,\"Icarus Verilog\"" << endl;
      if (des->get_flag("part") != "")
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
 * This function makes an adder out of carry logic symbols. It makes
 * as many 2 bit adders as are possible, then the top bit is made into
 * a 1-bit adder (with carry in) in the F unit. The low carry is
 * initialized with the FORCE-0 configuration of a carry unit below
 * the 0 bit. This takes up the carry logic of the CLB below, but not
 * the G function.
 *
 * References:
 *    XNF 6.1 Specification
 *    Application note XAPP 013
 */
void target_xnf::lpm_add_sub(ostream&os, const NetAddSub*gate)
{
      // Keep people from thinking this really works.
	//assert(0);
      unsigned width = gate->width();

      string name = mangle(gate->name());

      string name_add = name;
      string name_cy4 = name + "/CY";
      string name_cym = name + "/CM";
      string name_cout = name + "/COUT";

	/* Make the force-0 cary mode object to initialize the bottom
	   bits of the carry chain. Label this with the width instead
	   of the bit position so that symbols dont clash. */
      os << "SYM, " << name_cy4 << "<" << width << ">, CY4, "
	    "LIBVER=2.0.0" << endl;
      os << "    PIN, COUT, O, " << name_cout << "<" << width << ">" << endl;
      for (unsigned cn = 0 ;  cn < 8 ;  cn += 1) {
	    os << "    PIN, C" << cn << ", I, " << name << "/C"
	       << cn << "<" << width << ">" << endl;
      }
      os << "END" << endl;

      os << "SYM, " << name_cym << "<" << width << ">, CY4_37, "
	    "CYMODE=FORCE-0" << endl;
      for (unsigned cn = 0 ;  cn < 8 ;  cn += 1) {
	    os << "    PIN, C" << cn << ", O, " << name << "/C"
	       << cn << "<" << width << ">" << endl;
      }
      os << "END" << endl;

	/* Now make the 2 bit adders that chain from the cin
	   initializer and up. Safe the tail bit (if there is one) for
	   later. */
      for (unsigned idx = 0 ;  idx < (width&~1) ;  idx += 2) {
	    os << "SYM, " << name_cy4 << "<" << idx << ">, CY4, "
		  "LIBVER=2.0.0" << endl;

	    draw_pin(os, "A0", gate->pin_DataA(idx+0));
	    draw_pin(os, "B0", gate->pin_DataB(idx+0));
	    draw_pin(os, "A1", gate->pin_DataA(idx+1));
	    draw_pin(os, "B1", gate->pin_DataB(idx+1));

	      // Connect the Cout0 to a signal so that I can connect
	      // it to the adder.
	    os << "    PIN, COUT0, O, " << name_cout << "<" << idx <<
		  ">" << endl;

	      // Connect the Cout to the next Cin
	    if ((idx+2) < width)
		  os << "    PIN, COUT, O, " << name_cout << "<" <<
			(idx+1) << ">" << endl;

	      // Connect the Cin to the previous Cout
	    if (idx > 0)
		  os  << "    PIN, CIN, I, " << name_cout << "<" <<
			(idx-1) << ">" << endl;
	    else
		  os  << "    PIN, CIN, I, " << name_cout << "<" <<
			(width) << ">" << endl;


	    for (unsigned cn = 0 ;  cn < 8 ;  cn += 1) {
		  os << "    PIN, C" << cn << ", I, " << name << "/C"
		     << cn << "<" << idx << ">" << endl;
	    }
	    os << "END" << endl;

	    os << "SYM, " << name_cym << "<" << idx << ">, ";
	    os << "CY4_02, CYMODE=ADD-FG-CI" << endl;
	    for (unsigned cn = 0 ;  cn < 8 ;  cn += 1) {
		  os << "    PIN, C" << cn << ", O, " << name << "/C"
		     << cn << "<" << idx << ">" << endl;
	    }
	    os << "END" << endl;

	      // Draw the first bit adder. We only need to pick up the
	      // carry if we are not the 0 bit. (We know it is 0).
	    os << "SYM, " << name_add << "<" << (idx+0) << ">, XOR, "
		  "LIBVER=2.0.0" << endl;
	    draw_pin(os, "O",  gate->pin_Result(idx));
	    draw_pin(os, "I0", gate->pin_DataA(idx));
	    draw_pin(os, "I1", gate->pin_DataB(idx));
	    if (idx > 0) {
		  os << "    PIN, I2, I, " << name_cout << "<" <<
			(idx-1) << ">" << endl;
	    }
	    os << "END" << endl;

	      // Draw the second bit adder. This gets the carry from
	      // the first bit CY4.
	    os << "SYM, " << name_add << "<" << (idx+1) << ">, XOR, "
		  "LIBVER=2.0.0" << endl;
	    draw_pin(os, "O",  gate->pin_Result(idx+1));
	    draw_pin(os, "I0", gate->pin_DataA(idx+1));
	    draw_pin(os, "I1", gate->pin_DataB(idx+1));
	    os << "    PIN, I2, I, " << name_cout << "<" << idx << ">"
	       << endl;
	    os << "END" << endl;

      }

	/* If there is a tail bit, draw it as a single bit adder that
	   takes its carry in from the previously drawn adder. */
      if (width%2) {
	    os << "SYM, " << name_cy4 << "<" << (width-1) << ">, CY4, "
		  "LIBVER=2.0.0" << endl;

	    draw_pin(os, "A0", gate->pin_DataA(width-1));
	    draw_pin(os, "B0", gate->pin_DataB(width-1));

	    if (width > 2)
		  os  << "    PIN, CIN, I, " << name_cout << "<" <<
			(width-2) << ">" << endl;


	    for (unsigned cn = 0 ;  cn < 8 ;  cn += 1) {
		  os << "    PIN, C" << cn << ", I, " << name << "/C"
		     << cn << "<" << (width-1) << ">" << endl;
	    }
	    os << "END" << endl;

	    os << "SYM, " << name_cym << "<" << (width-1) << ">, ";
	    os << "CY4_01, CYMODE=ADD-F-CI" << endl;
	    for (unsigned cn = 0 ;  cn < 8 ;  cn += 1) {
		  os << "    PIN, C" << cn << ", O, " << name << "/C"
		     << cn << "<" << (width-1) << ">" << endl;
	    }
	    os << "END" << endl;

	    os << "SYM, " << name_add << "<" << (width-1) << ">, XOR, "
		  "LIBVER=2.0.0" << endl;
	    draw_pin(os, "O",  gate->pin_Result(width-1));
	    draw_pin(os, "I0", gate->pin_DataA(width-1));
	    draw_pin(os, "I1", gate->pin_DataB(width-1));
	    os << "    PIN, I2, I, " << name_cout << "<" << (width-2) << ">"
	       << endl;
	    os << "END" << endl;
      }

}

void target_xnf::lpm_ff(ostream&os, const NetFF*net)
{
      string type = net->attribute("LPM_FFType");
      if (type == "") type = "DFF";

	// XXXX For now, only support DFF
      assert(type == "DFF");
	// XXXX For now, I do not now how to deal with XNF-LCA attributes.
      assert(net->attribute("XNF-LCA") == "");

      for (unsigned idx = 0 ;  idx < net->width() ;  idx += 1) {

	    os << "SYM, " << mangle(net->name()) << "<" << idx
	       << ">, DFF, LIBVER=2.0.0" << endl;
	    draw_pin(os, "Q", net->pin_Q(idx));
	    draw_pin(os, "D", net->pin_Data(idx));

	    if (net->attribute("Clock:LPM_Polarity") == "INVERT")
		  draw_pin(os, "~C", net->pin_Clock());
	    else
		  draw_pin(os, "C", net->pin_Clock());

	    if (count_outputs(net->pin_Enable()) > 0)
		  draw_pin(os, "CE", net->pin_Enable());

	    os << "END" << endl;
      }
}

/*
 * Generate an LPM_MUX.
 *
 *   XXXX NOTE: For now, this only supports combinational LPM_MUX
 *   devices that have a single select input. These are typically
 *   generated from ?: expressions.
 */
void target_xnf::lpm_mux(ostream&os, const NetMux*net)
{
      assert(net->sel_width() == 1);
      assert(net->size() == 2);

      for (unsigned idx = 0 ;  idx < net->width() ;  idx += 1) {

	    os << "SYM, " << mangle(net->name()) << "<" << idx << ">,"
	       << " EQN, EQN=(I0 * I2) + (~I0 * I1)" << endl;

	    draw_pin(os, "I0", net->pin_Sel(0));
	    draw_pin(os, "I1", net->pin_Data(idx,0));
	    draw_pin(os, "I2", net->pin_Data(idx,1));
	    draw_pin(os, "O",  net->pin_Result(idx));

	    os << "END" << endl;
      }

}

void target_xnf::net_const(ostream&os, const NetConst*c)
{
      verinum::V v=c->value();
      assert(v==verinum::V0 || v==verinum::V1);
      const NetObj::Link& lnk = c->pin(0);
      // Code parallels draw_pin above, some smart c++ guru should
      // find a way to make a method out of this.
      unsigned cpin;
      const NetObj*cur;
      for (lnk.next_link(cur, cpin)
		 ; cur->pin(cpin) != lnk
		 ; cur->pin(cpin).next_link(cur, cpin)) {

	    const NetNet*sig = dynamic_cast<const NetNet*>(cur);
	    if (sig) {
		os << "PWR, " << v << ", " << mangle(sig->name());
		if (sig->pin_count() > 1)
                        os << "<" << cpin << ">";
		os << endl;
	    }
      }
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

void target_xnf::bufz(ostream&os, const NetBUFZ*net)
{
      static int warned_once=0;
      if (!warned_once) {
	    cerr << "Warning: BUFZ object found for xnf output."
		  " Try -Fnobufz." << endl;
	    warned_once=1;
      }
      os << "SYM, " << mangle(net->name()) << ", BUF, LIBVER=2.0.0" << endl;
      assert(net->pin_count() == 2);
      draw_pin(os, "O", net->pin(0));
      draw_pin(os, "I", net->pin(1));
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
 * Revision 1.14  1999/11/05 18:43:12  steve
 *  fix syntax of EQN record.
 *
 * Revision 1.13  1999/11/05 07:10:45  steve
 *  Include the obvious XOR gates in the adders.
 *
 * Revision 1.12  1999/11/05 04:40:40  steve
 *  Patch to synthesize LPM_ADD_SUB from expressions,
 *  Thanks to Larry Doolittle. Also handle constants
 *  in expressions.
 *
 *  Synthesize adders in XNF, based on a patch from
 *  Larry. Accept synthesis of constants from Larry
 *  as is.
 *
 * Revision 1.11  1999/11/04 03:53:26  steve
 *  Patch to synthesize unary ~ and the ternary operator.
 *  Thanks to Larry Doolittle <LRDoolittle@lbl.gov>.
 *
 *  Add the LPM_MUX device, and integrate it with the
 *  ternary synthesis from Larry. Replace the lpm_mux
 *  generator in t-xnf.cc to use XNF EQU devices to
 *  put muxs into function units.
 *
 *  Rewrite elaborate_net for the PETernary class to
 *  also use the LPM_MUX device.
 *
 * Revision 1.10  1999/11/02 04:55:34  steve
 *  Add the synthesize method to NetExpr to handle
 *  synthesis of expressions, and use that method
 *  to improve r-value handling of LPM_FF synthesis.
 *
 *  Modify the XNF target to handle LPM_FF objects.
 *
 * Revision 1.9  1999/08/25 22:22:08  steve
 *  handle bufz in XNF backend.
 *
 * Revision 1.8  1999/08/18 04:00:02  steve
 *  Fixup spelling and some error messages. <LRDoolittle@lbl.gov>
 *
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

