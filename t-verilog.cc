/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: t-verilog.cc,v 1.15 2000/09/26 01:35:42 steve Exp $"
#endif

/*
 * This rather interesting target generates Verilog from the
 * design. This is interesting since by the time I get here,
 * elaboration and generic optimizations have been performed. This is
 * useful for feeding optimized designs to other tools, or the human
 * for debugging.
 */

# include  <fstream>
# include  <iomanip>
# include  <strstream>
# include  <typeinfo>
# include  "target.h"

extern const struct target tgt_verilog;

class target_verilog : public target_t {
    public:
      virtual bool start_design(const Design*);
      virtual void signal(const NetNet*);
      virtual void logic(const NetLogic*);
      virtual bool bufz(const NetBUFZ*);

      virtual bool process(const NetProcTop*);
      virtual bool proc_block(const NetBlock*);
      virtual bool proc_delay(const NetPDelay*);
      virtual void proc_stask(const NetSTask*);
      virtual void end_design(const Design*);

    private:
      void start_process(const NetProcTop*);
      void end_process(const NetProcTop*);
      unsigned indent_;

      ofstream out;

      void emit_expr_(ostream&os, const NetExpr*);
};


/*
 * The output of the design starts here. The emit operation calls the
 * design_start and design_end target methods around the scan of the
 * design.  Targets can use these hooks to generate header or footer
 * information if desired.
 */
bool target_verilog::start_design(const Design*des)
{
      out.open(des->get_flag("-o").c_str(), ios::out | ios::trunc);

      indent_ = 0;
      out << "module " << "main" << ";" << endl;
      return true;
}

/*
 * Emit first iterates over all the signals. This gives the target a
 * chance to declare signal variables before the network is assembled
 * or behaviors are written.
 */
void target_verilog::signal(const NetNet*net)
{
      out << "    " << net->type();
      if (net->pin_count() > 1)
	    out << " [" << net->msb() << ":" << net->lsb() << "]";

      if (net->rise_time())
	    out << " #" << net->rise_time();

      out << " " << mangle(net->name()) << ";" << endl;
}

void target_verilog::logic(const NetLogic*net)
{
      switch (net->type()) {

	  case NetLogic::AND:
	    out << "    and";
	    break;
	  case NetLogic::NAND:
	    out << "    nand";
	    break;
	  case NetLogic::NOR:
	    out << "    nor";
	    break;
	  case NetLogic::NOT:
	    out << "    not";
	    break;
	  case NetLogic::OR:
	    out << "    or";
	    break;
	  case NetLogic::XNOR:
	    out << "    xnor";
	    break;
	  case NetLogic::XOR:
	    out << "    xor";
	    break;
      }

      out << " #" << net->rise_time() << " " << mangle(net->name()) << "(";

      unsigned sidx;
      const NetNet*sig = find_link_signal(net, 0, sidx);
      out << mangle(sig->name()) << "[" << sidx << "]";
      for (unsigned idx = 1 ;  idx < net->pin_count() ;  idx += 1) {
	    sig = find_link_signal(net, idx, sidx);
	    assert(sig);
	    out << ", " << mangle(sig->name()) << "[" << sidx << "]";
      }
      out << ");" << endl;
}

bool target_verilog::bufz(const NetBUFZ*net)
{
      assert( net->pin_count() == 2 );
      out << "    assign ";

      unsigned sidx;
      const NetNet*sig = find_link_signal(net, 0, sidx);
      out << mangle(sig->name()) << "[" << sidx << "] = ";

      sig = find_link_signal(net, 1, sidx);
      out << mangle(sig->name()) << "[" << sidx << "];" << endl;
      return true;
}

bool target_verilog::process(const NetProcTop*top)
{
      start_process(top);
      bool rc = top->statement()->emit_proc(this);
      end_process(top);
      return rc;
}

void target_verilog::start_process(const NetProcTop*proc)
{
      switch (proc->type()) {
	  case NetProcTop::KINITIAL:
	    out << "    initial" << endl;
	    break;
	  case NetProcTop::KALWAYS:
	    out << "    always" << endl;
	    break;
      }

      indent_ = 6;
}

void target_verilog::end_process(const NetProcTop*)
{
}

void target_verilog::emit_expr_(ostream&os, const NetExpr*expr)
{

      if (const NetEConst*pp = dynamic_cast<const NetEConst*>(expr)) {

	    os << pp->value();

      } else if (const NetEUnary*pp = dynamic_cast<const NetEUnary*>(expr)) {

	    os << pp->op() << "(";
	    emit_expr_(os, pp->expr());
	    os << ")";

      } else {
	    os << "(huh?)";
      }
}

bool target_verilog::proc_block(const NetBlock*net)
{
      out << setw(indent_) << "" << "begin" << endl;
      indent_ += 4;
      net->emit_recurse(this);
      indent_ -= 4;
      out << setw(indent_) << "" << "end" << endl;
      return true;
}

bool target_verilog::proc_delay(const NetPDelay*net)
{
      out << setw(indent_) << "" << "#" << net->delay() << endl;

      indent_ += 4;
      bool flag = net->emit_proc_recurse(this);
      indent_ -= 4;
      return flag;
}


static void vtask_parm(ostream&os, const NetExpr*ex)
{
      if (const NetEConst*pp = dynamic_cast<const NetEConst*>(ex)) {
	    if (pp->value().is_string())
		  os << "\"" << pp->value().as_string() << "\"";
	    else
		  os << pp->value();

      } else {
      }
}

void target_verilog::proc_stask(const NetSTask*net)
{
      out << setw(indent_) << "" << net->name();
      if (net->nparms() > 0) {
	    out << "(";
	    vtask_parm(out, net->parm(0));
	    for (unsigned idx = 1 ;  idx < net->nparms() ;  idx += 1) {
		  out << ", ";
		  vtask_parm(out, net->parm(idx));
	    }
	    out << ")";
      }

      out << ";" << endl;
}


/*
 * All done with the design. Flush any output that I've been holding
 * off, and write the footers for the target.
 */
void target_verilog::end_design(const Design*)
{
      out << "endmodule" << endl;
}

static target_verilog tgt_verilog_obj;

const struct target tgt_verilog = {
      "verilog",
      &tgt_verilog_obj
};

/*
 * $Log: t-verilog.cc,v $
 * Revision 1.15  2000/09/26 01:35:42  steve
 *  Remove the obsolete NetEIdent class.
 *
 * Revision 1.14  2000/08/14 04:39:57  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.13  2000/08/09 03:43:45  steve
 *  Move all file manipulation out of target class.
 *
 * Revision 1.12  2000/08/08 01:50:42  steve
 *  target methods need not take a file stream.
 *
 * Revision 1.11  2000/07/29 16:21:08  steve
 *  Report code generation errors through proc_delay.
 *
 * Revision 1.10  2000/04/12 04:23:58  steve
 *  Named events really should be expressed with PEIdent
 *  objects in the pform,
 *
 *  Handle named events within the mix of net events
 *  and edges. As a unified lot they get caught together.
 *  wait statements are broken into more complex statements
 *  that include a conditional.
 *
 *  Do not generate NetPEvent or NetNEvent objects in
 *  elaboration. NetEvent, NetEvWait and NetEvProbe
 *  take over those functions in the netlist.
 */

