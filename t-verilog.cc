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
#ident "$Id: t-verilog.cc,v 1.6 1999/07/03 02:12:52 steve Exp $"
#endif

/*
 * This rather interesting target generates Verilog from the
 * design. This is interesting since by the time I get here,
 * elaboration and generic optimizations have been performed. This is
 * useful for feeding optimized designs to other tools, or the human
 * for debugging.
 */

# include  <iostream>
# include  <iomanip>
# include  <strstream>
# include  <typeinfo>
# include  "target.h"

extern const struct target tgt_verilog;

class target_verilog : public target_t {
    public:
      virtual void start_design(ostream&os, const Design*);
      virtual void signal(ostream&os, const NetNet*);
      virtual void logic(ostream&os, const NetLogic*);
      virtual void bufz(ostream&os, const NetBUFZ*);
      virtual void start_process(ostream&os, const NetProcTop*);
      virtual void proc_block(ostream&os, const NetBlock*);
      virtual void proc_delay(ostream&os, const NetPDelay*);
      virtual void proc_event(ostream&os, const NetPEvent*);
      virtual void proc_stask(ostream&os, const NetSTask*);
      virtual void end_design(ostream&os, const Design*);
    private:
      unsigned indent_;

      void emit_expr_(ostream&os, const NetExpr*);
};


/*
 * The output of the design starts here. The emit operation calls the
 * design_start and design_end target methods around the scan of the
 * design.  Targets can use these hooks to generate header or footer
 * information if desired.
 */
void target_verilog::start_design(ostream&os, const Design*)
{
      indent_ = 0;
      os << "module " << "main" << ";" << endl;
}

/*
 * Emit first iterates over all the signals. This gives the target a
 * chance to declare signal variables before the network is assembled
 * or behaviors are written.
 */
void target_verilog::signal(ostream&os, const NetNet*net)
{
      os << "    " << net->type();
      if (net->pin_count() > 1)
	    os << " [" << net->msb() << ":" << net->lsb() << "]";

      if (net->delay1())
	    os << " #" << net->delay1();

      os << " " << mangle(net->name()) << ";" << endl;
}

void target_verilog::logic(ostream&os, const NetLogic*net)
{
      switch (net->type()) {

	  case NetLogic::AND:
	    os << "    and";
	    break;
	  case NetLogic::NAND:
	    os << "    nand";
	    break;
	  case NetLogic::NOR:
	    os << "    nor";
	    break;
	  case NetLogic::NOT:
	    os << "    not";
	    break;
	  case NetLogic::OR:
	    os << "    or";
	    break;
	  case NetLogic::XNOR:
	    os << "    xnor";
	    break;
	  case NetLogic::XOR:
	    os << "    xor";
	    break;
      }

      os << " #" << net->delay1() << " " << mangle(net->name()) << "(";

      unsigned sidx;
      const NetNet*sig = find_link_signal(net, 0, sidx);
      os << mangle(sig->name()) << "[" << sidx << "]";
      for (unsigned idx = 1 ;  idx < net->pin_count() ;  idx += 1) {
	    sig = find_link_signal(net, idx, sidx);
	    assert(sig);
	    os << ", " << mangle(sig->name()) << "[" << sidx << "]";
      }
      os << ");" << endl;
}

void target_verilog::bufz(ostream&os, const NetBUFZ*net)
{
      assert( net->pin_count() == 2 );
      os << "    assign ";

      unsigned sidx;
      const NetNet*sig = find_link_signal(net, 0, sidx);
      os << mangle(sig->name()) << "[" << sidx << "] = ";

      sig = find_link_signal(net, 1, sidx);
      os << mangle(sig->name()) << "[" << sidx << "];" << endl;
}

void target_verilog::start_process(ostream&os, const NetProcTop*proc)
{
      switch (proc->type()) {
	  case NetProcTop::KINITIAL:
	    os << "    initial" << endl;
	    break;
	  case NetProcTop::KALWAYS:
	    os << "    always" << endl;
	    break;
      }

      indent_ = 6;
}

void target_verilog::emit_expr_(ostream&os, const NetExpr*expr)
{

      if (const NetEConst*pp = dynamic_cast<const NetEConst*>(expr)) {

	    os << pp->value();

      } else if (const NetEIdent*pp = dynamic_cast<const NetEIdent*>(expr)) {

	    os << mangle(pp->name());

      } else if (const NetEUnary*pp = dynamic_cast<const NetEUnary*>(expr)) {

	    os << pp->op() << "(";
	    emit_expr_(os, pp->expr());
	    os << ")";

      } else {
	    os << "(huh?)";
      }
}

void target_verilog::proc_block(ostream&os, const NetBlock*net)
{
      os << setw(indent_) << "" << "begin" << endl;
      indent_ += 4;
      net->emit_recurse(os, this);
      indent_ -= 4;
      os << setw(indent_) << "" << "end" << endl;
}

void target_verilog::proc_delay(ostream&os, const NetPDelay*net)
{
      os << setw(indent_) << "" << "#" << net->delay() << endl;

      indent_ += 4;
      net->emit_proc_recurse(os, this);
      indent_ -= 4;
}

void target_verilog::proc_event(ostream&os, const NetPEvent*net)
{
      os << setw(indent_) << "" << "@";

#if 0
      unsigned sidx;
      const NetNet*sig = find_link_signal(net, 0, sidx);

      switch (net->edge()) {
	  case NetNEvent::ANYEDGE:
	    os << mangle(sig->name()) << "[" << sidx << "]";
	    break;
	  case NetNEvent::POSEDGE:
	    os << "(posedge " << mangle(sig->name()) << "[" << sidx << "])";
	    break;
	  case NetNEvent::NEGEDGE:
	    os << "(negedge " << mangle(sig->name()) << "[" << sidx << "])";
	    break;
      }
#else
      os << endl;
      os << "#error \"proc_event temporarily out of order\"";
#endif
      os << endl;

      indent_ += 4;
      net->emit_proc_recurse(os, this);
      indent_ -= 4;
}

static void vtask_parm(ostream&os, const NetExpr*ex)
{
      if (const NetEConst*pp = dynamic_cast<const NetEConst*>(ex)) {
	    if (pp->value().is_string())
		  os << "\"" << pp->value().as_string() << "\"";
	    else
		  os << pp->value();

      } else if (const NetEIdent*pp = dynamic_cast<const NetEIdent*>(ex)) {
	    os << mangle(pp->name());

      } else {
      }
}

void target_verilog::proc_stask(ostream&os, const NetSTask*net)
{
      os << setw(indent_) << "" << net->name();
      if (net->nparms() > 0) {
	    os << "(";
	    vtask_parm(os, net->parm(0));
	    for (unsigned idx = 1 ;  idx < net->nparms() ;  idx += 1) {
		  os << ", ";
		  vtask_parm(os, net->parm(idx));
	    }
	    os << ")";
      }

      os << ";" << endl;
}


/*
 * All done with the design. Flush any output that I've been holding
 * off, and write the footers for the target.
 */
void target_verilog::end_design(ostream&os, const Design*)
{
      os << "endmodule" << endl;
}

static target_verilog tgt_verilog_obj;

const struct target tgt_verilog = {
      "verilog",
      &tgt_verilog_obj
};

/*
 * $Log: t-verilog.cc,v $
 * Revision 1.6  1999/07/03 02:12:52  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.5  1999/06/13 16:30:06  steve
 *  Unify the NetAssign constructors a bit.
 *
 * Revision 1.4  1999/05/01 02:57:53  steve
 *  Handle much more complex event expressions.
 *
 * Revision 1.3  1998/12/01 00:42:15  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.2  1998/11/23 00:20:23  steve
 *  NetAssign handles lvalues as pin links
 *  instead of a signal pointer,
 *  Wire attributes added,
 *  Ability to parse UDP descriptions added,
 *  XNF generates EXT records for signals with
 *  the PAD attribute.
 *
 * Revision 1.1  1998/11/03 23:29:05  steve
 *  Introduce verilog to CVS.
 *
 */

