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
#ident "$Id: t-xnf.cc,v 1.22 1999/12/17 03:38:46 steve Exp $"
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
 *   ncf=<path>
 *      Specify the path to a NCF file. This is an OUTPUT file into
 *      which the code generator will write netlist constraints that
 *      relate to pin assignments, CLB placement, etc. If this flag is
 *      not given, no NCF file will be written.
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
# include  <fstream>
# include  <strstream>

class target_xnf  : public target_t {

    public:
      void start_design(ostream&os, const Design*);
      void end_design(ostream&os, const Design*);
      void memory(ostream&os, const NetMemory*);
      void signal(ostream&os, const NetNet*);

      void lpm_add_sub(ostream&os, const NetAddSub*);
      void lpm_ff(ostream&os, const NetFF*);
      void lpm_mux(ostream&os, const NetMux*);
      void lpm_ram_dq(ostream&os, const NetRamDq*);

      void net_const(ostream&os, const NetConst*);
      void logic(ostream&os, const NetLogic*);
      void bufz(ostream&os, const NetBUFZ*);
      void udp(ostream&os,  const NetUDP*);

    private:
      static string mangle(const string&);
      static string choose_sig_name(const NetObj::Link*lnk);
      static void draw_pin(ostream&os, const string&name,
			   const NetObj::Link&lnk);
      static void draw_sym_with_lcaname(ostream&os, string lca,
					const NetNode*net);
      static void draw_xor(ostream&os, const NetAddSub*, unsigned idx);
      enum adder_type {FORCE0, LOWER, DOUBLE, LOWER_W_CO, EXAMINE_CI };
      static void draw_carry(ostream&os, const NetAddSub*, unsigned idx,
			     enum adder_type);

      ofstream ncf_;
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

/*
 * This method takes a signal and pin number as a nexus. Scan the
 * nexus to decide which name to use if there are lots of attached
 * signals.
 */
string target_xnf::choose_sig_name(const NetObj::Link*lnk)
{
      const NetNet*sig = dynamic_cast<const NetNet*>(lnk->get_obj());
      unsigned pin = lnk->get_pin();

      for (const NetObj::Link*cur = lnk->next_link()
		 ;  cur != lnk ;  cur = cur->next_link()) {

	    const NetNet*cursig = dynamic_cast<const NetNet*>(cur->get_obj());
	    if (cursig == 0)
		  continue;

	    if (sig == 0) {
		  sig = cursig;
		  pin = cur->get_pin();
		  continue;
	    }

	    if ((cursig->pin_count() == 1) && (sig->pin_count() > 1))
		  continue;

	    if ((cursig->pin_count() > 1) && (sig->pin_count() == 1)) {
		  sig = cursig;
		  pin = cur->get_pin();
		  continue;
	    }

	    if (cursig->local_flag() && !sig->local_flag())
		  continue;

	    if (cursig->name() < sig->name())
		  continue;

	    sig = cursig;
	    pin = cur->get_pin();
      }

      assert(sig);
      ostrstream tmp;
      tmp << mangle(sig->name());
      if (sig->pin_count() > 1)
	    tmp << "<" << pin << ">";
      tmp << ends;

      return tmp.str();
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

      os << "    PIN, " << use_name << ", " << type << ", " <<
	    choose_sig_name(&lnk);
      if (inv) os << ",,INV";
      os << endl;
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

      for (idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    string usename = scrape_pin_name(lca);
	    if (usename == "") continue;
	    draw_pin(os, usename, net->pin(idx));
      }

      os << "END" << endl;
}

void target_xnf::start_design(ostream&os, const Design*des)
{
      string ncfpath = des->get_flag("ncf");
      if (ncfpath != "")
	    ncf_.open(ncfpath.c_str());

      os << "LCANET,6" << endl;
      os << "PROG,verilog,0.2PRE,\"Icarus Verilog\"" << endl;
      ncf_ << "# Generated by Icarus Verilog 0.2PRE" << endl;

      if (des->get_flag("part") != "") {
	    os << "PART," << des->get_flag("part") << endl;
	    ncf_ << "CONFIG PART=" << des->get_flag("part") << ";" << endl;
      }
}

void target_xnf::end_design(ostream&os, const Design*)
{
      os << "EOF" << endl;
      ncf_.close();
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
 * Memories are handled by the lpm_ram_dq method, so there is nothing
 * to do here.
 */
void target_xnf::memory(ostream&, const NetMemory*)
{
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

      ncf_ << "# Assignment to pin " << num << " (DIR=" << dir <<
	    ") by $attribute(" << net->name() << ", \"PAD\", \"" <<
	    pad << "\")" << endl;
      ncf_ << "NET " << mangle(net->name()) << " LOC=P" << num << ";"
	   << endl;
}

void target_xnf::draw_xor(ostream &os, const NetAddSub*gate, unsigned idx)
{
      string name = mangle(gate->name());
      string name_add = name;
      string name_cout = name + "/COUT";

	      // We only need to pick up the
	      // carry if we are not the 0 bit. (We know it is 0).
      os << "SYM, " << name_add << "<" << (idx+0) << ">, XOR, "
	  "LIBVER=2.0.0" << endl;
      draw_pin(os, "O",  gate->pin_Result(idx));
      draw_pin(os, "I0", gate->pin_DataA(idx));
      draw_pin(os, "I1", gate->pin_DataB(idx));
      if (idx > 0) {
	  os << "    PIN, I2, I, " << name_cout << "<" <<
		idx << ">" << endl;
      }
      os << "END" << endl;
}

void target_xnf::draw_carry(ostream &os, const NetAddSub*gate, unsigned idx,
      enum adder_type type)
{
      string name = mangle(gate->name());

      string name_cy4 = name + "/CY";
      string name_cym = name + "/CM";
      string name_cout = name + "/COUT";

      os << "SYM, " << name_cy4 << "<" << idx << ">, CY4, "
	    "LIBVER=2.0.0" << endl;

      // Less significant bit addends, if any
      if ( type == LOWER || type == DOUBLE || type == LOWER_W_CO ) {
	    draw_pin(os, "A0", gate->pin_DataA(idx));
	    draw_pin(os, "B0", gate->pin_DataB(idx));
      }

      // More significant bit addends, if any
      if ( type == DOUBLE ) {
	    draw_pin(os, "A1", gate->pin_DataA(idx+1));
	    draw_pin(os, "B1", gate->pin_DataB(idx+1));
      }

      // All but FORCE0 cells have carry input
      if ( type != FORCE0 ) {
	  os  << "    PIN, CIN, I, " << name_cout << "<" << idx << ">" << endl;
      }

      // Connect the Cout0 to a signal so that I can connect
      // it to the adder.
      switch (type) {
	  case LOWER:
	  case DOUBLE:
	    os << "    PIN, COUT0, O, " << name_cout << "<" << (idx+1) <<
		  ">" << endl;
	    break;
	  case EXAMINE_CI:
	  case LOWER_W_CO:
	    draw_pin(os, "COUT0", gate->pin_Cout());
	    break;
      }

      // Connect the Cout, this will connect to the next Cin
      if ( type == FORCE0 || type == DOUBLE ) {
	    unsigned int to = (type==FORCE0)?(0):(idx+2);
	    os << "    PIN, COUT, O, " << name_cout << "<" << to <<
		  ">" << endl;
      }

      // These are the mode inputs from the CY_xx pseudo-device
      for (unsigned cn = 0 ;  cn < 8 ;  cn += 1) {
	    os << "    PIN, C" << cn << ", I, " << name << "/C"
	       << cn << "<" << (idx) << ">" << endl;
      }
      os << "END" << endl;

      // On to the CY_xx pseudo-device itself
      os << "SYM, " << name_cym << "<" << (idx) << ">, ";
      switch (type) {
	  case FORCE0:
	    os << "CY4_37, CYMODE=FORCE-0" << endl;
	    break;
	  case LOWER:
	    os << "CY4_01, CYMODE=ADD-F-CI" << endl;
	    break;
	  case LOWER_W_CO:
	    os << "CY4_01, CYMODE=ADD-F-CI" << endl;
	    break;
	  case DOUBLE:
	    os << "CY4_02, CYMODE=ADD-FG-CI" << endl;
	    break;
	  case EXAMINE_CI:
	    os << "CY4_42, CYMODE-EXAMINE-CI" << endl;
	    break;
      }
      for (unsigned cn = 0 ;  cn < 8 ;  cn += 1) {
	    os << "    PIN, C" << cn << ", O, " << name << "/C"
	       << cn << "<" << (idx) << ">" << endl;
      }
      os << "END" << endl;
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
 *    Application note XAPP013
 *    Xilinx Libraries Guide, Chapter 12
 */
void target_xnf::lpm_add_sub(ostream&os, const NetAddSub*gate)
{
      unsigned width = gate->width();

	/* Make the force-0 carry mode object to initialize the bottom
	   bits of the carry chain. Label this with the width instead
	   of the bit position so that symbols don't clash. */

      draw_carry(os, gate, width+1, FORCE0);


	/* Now make the 2 bit adders that chain from the cin
	   initializer and up. Save the tail bits for later. */
      for (unsigned idx = 0 ;  idx < width-2 ;  idx += 2)
	    draw_carry(os, gate, idx, DOUBLE);

	/* Always have one or two tail bits. The situation gets a
	   little tricky if we want the carry output, so handle that
	   here.

	   If the carry-out is connected, and there are an even number
	   of data bits, we need to see the cout from the CLB. This is
	   done by configuring the top CLB CY device as ADD-FG-CI (to
	   activate cout) and create an extra CLB CY device on top of
	   the carry chain configured EXAMINE-CI to put the carry into
	   the G function block.

	   IF the carry-out is connected and there are an odd number
	   of data bits, then the top CLB can be configured to carry
	   the top bit in the F unit and deliver the carry out through
	   the G unit.

	   If the carry-out is not connected, then configure this top
	   CLB as ADD-F-CI. The draw_xor for the top bit will include
	   the F carry if needed. */

      if (gate->pin_Cout().is_linked()) {
	    if (width%2 == 0) {
		  draw_carry(os, gate, width-2, DOUBLE);
		  draw_carry(os, gate, width, EXAMINE_CI);
	    } else {
		  draw_carry(os, gate, width-1, LOWER_W_CO);
	    }

      } else {
	    if (width%2 == 0)
		  draw_carry(os, gate, width-2, LOWER);
	    else
		  draw_carry(os, gate, width-1, LOWER);
      }

	/* Now draw all the single bit (plus carry in) adders from XOR
	   gates. This puts the F and G units to use. */
      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    draw_xor(os, gate, idx);

}

void target_xnf::lpm_ff(ostream&os, const NetFF*net)
{
      string type = net->attribute("LPM_FFType");
      if (type == "") type = "DFF";

	// XXXX For now, only support DFF
      assert(type == "DFF");

      string lcaname = net->attribute("XNF-LCA");
      if (lcaname != "") {
	    draw_sym_with_lcaname(os, lcaname, net);
	    return;
      }

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

void target_xnf::lpm_ram_dq(ostream&os, const NetRamDq*ram)
{
      assert(ram->count_partners() == 1);

      const NetMemory*mem = ram->mem();

      for (unsigned idx = 0 ;  idx < ram->width() ;  idx += 1) {
	    os << "SYM, " << mangle(ram->name())
	       << "<" << idx << ">, RAMS" << endl;

	    draw_pin(os, "O", ram->pin_Q(idx));
	    draw_pin(os, "D", ram->pin_Data(idx));
	    draw_pin(os, "WE", ram->pin_WE());
	    draw_pin(os, "WCLK", ram->pin_InClock());
	    for (unsigned adr = 0 ;  adr < ram->awidth() ;  adr += 1) {
		  strstream tmp;
		  tmp << "A" << adr << ends;
		  draw_pin(os, tmp.str(), ram->pin_Address(adr));
	    }

	    os << "END" << endl;
      }
}

void target_xnf::net_const(ostream&os, const NetConst*c)
{
      for (unsigned idx = 0 ;  idx < c->pin_count() ;  idx += 1) {
	    verinum::V v=c->value(idx);
	    assert(v==verinum::V0 || v==verinum::V1);
	    const NetObj::Link& lnk = c->pin(idx);
	      // Code parallels draw_pin above, some smart c++ guru should
	      // find a way to make a method out of this.
	    unsigned cpin;
	    const NetObj*cur;

	    os << "    PWR, " << v << ", " << choose_sig_name(&lnk) << endl;
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
 * Revision 1.22  1999/12/17 03:38:46  steve
 *  NetConst can now hold wide constants.
 *
 * Revision 1.21  1999/12/16 18:54:32  steve
 *  Capture the carry out of carry-chain addition.
 *
 * Revision 1.20  1999/12/16 02:42:15  steve
 *  Simulate carry output on adders.
 *
 * Revision 1.19  1999/12/05 19:30:43  steve
 *  Generate XNF RAMS from synthesized memories.
 *
 * Revision 1.18  1999/11/19 03:02:25  steve
 *  Detect flip-flops connected to opads and turn
 *  them into OUTFF devices. Inprove support for
 *  the XNF-LCA attribute in the process.
 *
 * Revision 1.17  1999/11/17 18:52:09  steve
 *  Add algorithm for choosing nexus name from attached signals.
 *
 * Revision 1.16  1999/11/17 01:31:28  steve
 *  Clean up warnings that add_sub got from Alliance
 *
 * Revision 1.15  1999/11/06 04:51:42  steve
 *  Support writing some XNF things into an NCF file.
 *
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

