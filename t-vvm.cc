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
#ident "$Id: t-vvm.cc,v 1.14 1999/03/15 02:43:32 steve Exp $"
#endif

# include  <iostream>
# include  <strstream>
# include  <iomanip>
# include  <string>
# include  <typeinfo>
# include  "netlist.h"
# include  "target.h"

static string make_temp()
{
      static unsigned counter = 0;
      ostrstream str;
      str << "TMP" << counter << ends;
      counter += 1;
      return str.str();
}

class target_vvm : public target_t {
    public:
      virtual void start_design(ostream&os, const Design*);
      virtual void signal(ostream&os, const NetNet*);
      virtual void logic(ostream&os, const NetLogic*);
      virtual void bufz(ostream&os, const NetBUFZ*);
      virtual void udp(ostream&os, const NetUDP*);
      virtual void net_const(ostream&os, const NetConst*);
      virtual void net_esignal(ostream&os, const NetESignal*);
      virtual void net_pevent(ostream&os, const NetPEvent*);
      virtual void start_process(ostream&os, const NetProcTop*);
      virtual void proc_assign(ostream&os, const NetAssign*);
      virtual void proc_block(ostream&os, const NetBlock*);
      virtual void proc_case(ostream&os, const NetCase*net);
      virtual void proc_condit(ostream&os, const NetCondit*);
      virtual void proc_task(ostream&os, const NetTask*);
      virtual void proc_while(ostream&os, const NetWhile*);
      virtual void proc_event(ostream&os, const NetPEvent*);
      virtual void proc_delay(ostream&os, const NetPDelay*);
      virtual void end_process(ostream&os, const NetProcTop*);
      virtual void end_design(ostream&os, const Design*);

    private:
      void emit_gate_outputfun_(const NetNode*);

      ostrstream delayed;
      ostrstream init_code;
      ostrstream start_code;
      unsigned process_counter;
      unsigned thread_step_;
};


/*
 * This class emits code for the rvalue of a procedural
 * assignment. The expression is evaluated to fit the width
 * specified.
 */
class vvm_proc_rval  : public expr_scan_t {

    public:
      explicit vvm_proc_rval(ostream&os, unsigned i)
      : result(""), os_(os), indent_(i) { }

      string result;

    private:
      ostream&os_;
      unsigned indent_;

    private:
      virtual void expr_const(const NetEConst*);
      virtual void expr_ident(const NetEIdent*);
      virtual void expr_signal(const NetESignal*);
      virtual void expr_unary(const NetEUnary*);
      virtual void expr_binary(const NetEBinary*);
};

void vvm_proc_rval::expr_const(const NetEConst*expr)
{
      string tname = make_temp();
      os_ << setw(indent_) << "" << "vvm_bitset_t<" <<
	    expr->expr_width() << "> " << tname << ";" << endl;
      for (unsigned idx = 0 ;  idx < expr->expr_width() ;  idx += 1) {
	    os_ << setw(indent_) << "" << tname << "[" << idx << "] = ";
	    switch (expr->value().get(idx)) {
		case verinum::V0:
		  os_ << "V0";
		  break;
		case verinum::V1:
		  os_ << "V1";
		  break;
		case verinum::Vx:
		  os_ << "Vx";
		  break;
		case verinum::Vz:
		  os_ << "Vz";
		  break;
	    }
	    os_ << ";" << endl;
      }

      result = tname;
}

void vvm_proc_rval::expr_ident(const NetEIdent*expr)
{
      result = mangle(expr->name());
}

void vvm_proc_rval::expr_signal(const NetESignal*expr)
{
      result = mangle(expr->name()) + "_bits";
}

void vvm_proc_rval::expr_unary(const NetEUnary*expr)
{
      expr->expr()->expr_scan(this);
      string tname = make_temp();

      os_ << "        vvm_bitset_t<" << expr->expr_width() << "> "
	  << tname << " = ";
      switch (expr->op()) {
	  case '~':
	    os_ << "vvm_unop_not(" << result << ");"
		<< endl;
	    break;
	  case '!':
	    os_ << "vvm_unop_lnot(" << result << ");"
		<< endl;
	    break;
	  default:
	    cerr << "vvm: Unhandled unary op `" << expr->op() << "'"
		 << endl;
	    os_ << result << ";" << endl;
	    break;
      }

      result = tname;
}

void vvm_proc_rval::expr_binary(const NetEBinary*expr)
{
      expr->left()->expr_scan(this);
      string lres = result;

      expr->right()->expr_scan(this);
      string rres = result;

      result = make_temp();
      os_ << setw(indent_) << "" << "vvm_bitset_t<" <<
	    expr->expr_width() << ">" << result << ";" << endl;
      switch (expr->op()) {
	  case 'e':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_eq("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'n':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_ne("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'o':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_lor("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case '+':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_plus("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case '-':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_minus("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case '&':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_and("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case '|':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_or("
		<< lres << "," << rres << ");" << endl;
	    break;
	  default:
	    cerr << "vvm: Unhandled binary op `" << expr->op() << "': "
		 << *expr << endl;
	    os_ << lres << ";" << endl;
	    result = lres;
	    break;
      }
}

static string emit_proc_rval(ostream&os, unsigned indent, const NetExpr*expr)
{
      vvm_proc_rval scan (os, indent);
      expr->expr_scan(&scan);
      return scan.result;
}

class vvm_parm_rval  : public expr_scan_t {

    public:
      explicit vvm_parm_rval(ostream&os)
      : result(""), os_(os) { }

      string result;

    private:
      virtual void expr_const(const NetEConst*);
      virtual void expr_ident(const NetEIdent*);
      virtual void expr_signal(const NetESignal*);

    private:
      ostream&os_;
};

void vvm_parm_rval::expr_const(const NetEConst*expr)
{
      if (expr->value().is_string()) {
	    result = "\"";
	    result = result + expr->value().as_string() + "\"";
	    return;
      }
}

void vvm_parm_rval::expr_ident(const NetEIdent*expr)
{
      if (expr->name() == "$time") {
	    string res = make_temp();
	    os_ << "        vvm_calltf_parm " << res <<
		  "(vvm_calltf_parm::TIME);" << endl;
	    result = res;
      } else {
	    cerr << "Unhandled identifier: " << expr->name() << endl;
      }
}

void vvm_parm_rval::expr_signal(const NetESignal*expr)
{
      string res = make_temp();
      os_ << "        vvm_calltf_parm::SIG " << res << ";" << endl;
      os_ << "        " << res << ".bits = &" <<
	    mangle(expr->name()) << "_bits;" << endl;
      os_ << "        " << res << ".mon = &" <<
	    mangle(expr->name()) << ";" << endl;
      result = res;
}

static string emit_parm_rval(ostream&os, const NetExpr*expr)
{
      vvm_parm_rval scan (os);
      expr->expr_scan(&scan);
      return scan.result;
}

void target_vvm::start_design(ostream&os, const Design*mod)
{
      os << "# include \"vvm.h\"" << endl;
      os << "# include \"vvm_gates.h\"" << endl;
      os << "# include \"vvm_func.h\"" << endl;
      os << "# include \"vvm_calltf.h\"" << endl;
      os << "# include \"vvm_thread.h\"" << endl;
      process_counter = 0;

      init_code << "static void design_init(vvm_simulation&sim)" << endl;
      init_code << "{" << endl;

      start_code << "static void design_start(vvm_simulation&sim)" << endl;
      start_code << "{" << endl;
}

void target_vvm::end_design(ostream&os, const Design*mod)
{
      delayed << ends;
      os << delayed.str();

      init_code << "}" << endl << ends;
      os << init_code.str();

      start_code << "}" << endl << ends;
      os << start_code.str();

      os << "main()" << endl << "{" << endl;
      os << "      vvm_simulation sim;" << endl;
      os << "      design_init(sim);" << endl;
      os << "      design_start(sim);" << endl;

      for (unsigned idx = 0 ;  idx < process_counter ;  idx += 1)
	    os << "      thread" << (idx+1) << "_t thread_" <<
		  (idx+1) << "(&sim);" << endl;

      os << "      sim.run();" << endl;
      os << "}" << endl;
}

void target_vvm::signal(ostream&os, const NetNet*sig)
{

	/* Scan the signals of the vector, passing the initial value
	   to the inputs of all the connected devices. */
      for (unsigned idx = 0 ;  idx < sig->pin_count() ;  idx += 1) {
	    if (sig->get_ival(idx) == verinum::Vz)
		  continue;

	    for (const NetObj::Link*lnk = sig->pin(0).next_link()
		       ; (*lnk) != sig->pin(0) ;  lnk = lnk->next_link()) {
		  const NetNode*net;
		  if ((net = dynamic_cast<const NetNode*>(lnk->get_obj()))) {
			init_code << "      " <<
			      mangle(lnk->get_obj()->name()) <<
			      ".init(" << lnk->get_pin() << ", V" <<
			      sig->get_ival(idx) << ");" << endl;
		  }
	    }
      }
}

/*
 * This method handles writing output functions for gates that have a
 * single output (at pin 0). This writes the output_fun method into
 * the delayed stream to be emitted to the output file later.
 */
void target_vvm::emit_gate_outputfun_(const NetNode*gate)
{
      delayed << "static void " << mangle(gate->name()) <<
	    "_output_fun(vvm_simulation*sim, vvm_bit_t val)" <<
	    endl << "{" << endl;

	/* The output function connects to pin 0 of the netlist part
	   and causes the inputs that it is connected to to be set
	   with the new value. */

      const NetObj*cur;
      unsigned pin;
      gate->pin(0).next_link(cur, pin);
      while (cur != gate) {

	    if (dynamic_cast<const NetNet*>(cur)) {
		    // Skip signals
	    } else {

		  delayed << "      " << mangle(cur->name()) << ".set(sim, "
			  << pin << ", val);" << endl;
	    }

	    cur->pin(pin).next_link(cur, pin);
      }

      delayed << "}" << endl;
}

void target_vvm::logic(ostream&os, const NetLogic*gate)
{
      os << "static void " << mangle(gate->name()) <<
	    "_output_fun(vvm_simulation*, vvm_bit_t);" << endl;

      switch (gate->type()) {
	  case NetLogic::AND:
	    os << "static vvm_and" << "<" << gate->pin_count()-1 <<
		  "," << gate->delay1() << "> ";
	    break;
	  case NetLogic::BUFIF0:
	    os << "static vvm_bufif0<" << gate->delay1() << "> ";
	    break;
	  case NetLogic::BUFIF1:
	    os << "static vvm_bufif1<" << gate->delay1() << "> ";
	    break;
	  case NetLogic::NAND:
	    os << "static vvm_nand" << "<" << gate->pin_count()-1 <<
		  "," << gate->delay1() << "> ";
	    break;
	  case NetLogic::NOR:
	    os << "static vvm_nor" << "<" << gate->pin_count()-1 <<
		  "," << gate->delay1() << "> ";
	    break;
	  case NetLogic::NOT:
	    os << "static vvm_not" << "<" << gate->delay1() << "> ";
	    break;
	  case NetLogic::OR:
	    os << "static vvm_or" << "<" << gate->pin_count()-1 <<
		  "," << gate->delay1() << "> ";
	    break;
	  case NetLogic::XNOR:
	    os << "static vvm_xnor" << "<" << gate->pin_count()-1 <<
		  "," << gate->delay1() << "> ";
	    break;
	  case NetLogic::XOR:
	    os << "static vvm_xor" << "<" << gate->pin_count()-1 <<
		  "," << gate->delay1() << "> ";
	    break;
      }

      os << mangle(gate->name()) << "(&" <<
	    mangle(gate->name()) << "_output_fun);" << endl;

      emit_gate_outputfun_(gate);

      start_code << "      " << mangle(gate->name()) <<
	    ".start(&sim);" << endl;
}

void target_vvm::bufz(ostream&os, const NetBUFZ*gate)
{
      os << "static void " << mangle(gate->name()) <<
	    "_output_fun(vvm_simulation*, vvm_bit_t);" << endl;

      os << "static vvm_bufz " << mangle(gate->name()) << "(&" <<
	    mangle(gate->name()) << "_output_fun);" << endl;

      emit_gate_outputfun_(gate);
}

static string state_to_string(unsigned state, unsigned npins)
{
      static const char cur_table[3] = { '0', '1', 'x' };
      string res = "";
      for (unsigned idx = 0 ;  idx < npins ;  idx += 1) {
	    char cur = cur_table[state%3];
	    res = cur + res;
	    state /= 3;
      }

      return res;
}

void target_vvm::udp(ostream&os, const NetUDP*gate)
{
      assert(gate->pin_count() <= 9);
      assert(gate->is_sequential());
      unsigned states = 1;
      for (unsigned idx = 0 ;  idx < gate->pin_count() ;  idx += 1)
	    states *= 3;

      os << "static vvm_u32 " << mangle(gate->name()) << "_table[" <<
	    states << "] = {" << endl;
      os << hex;
      for (unsigned state = 0 ;  state < states ;  state += 1) {
	    string sstr = state_to_string(state, gate->pin_count());

	    unsigned long entry = 0;
	    for (unsigned idx = 1 ;  idx < sstr.length() ;  idx += 1) {
		  char o[2];
		  switch (sstr[idx]) {
		      case '0':
			o[0] = '1';
			o[1] = 'x';
			break;
		      case '1':
			o[0] = '0';
			o[1] = 'x';
			break;
		      case 'x':
			o[0] = '0';
			o[1] = '1';
			break;
		  }

		  o[0] = gate->table_lookup(sstr, o[0], idx);
		  o[1] = gate->table_lookup(sstr, o[1], idx);
		  entry <<= 2;
		  entry |= (o[0] == '0')? 0 : (o[0] == '1')? 1 : 2;
		  entry <<= 2;
		  entry |= (o[1] == '0')? 0 : (o[1] == '1')? 1 : 2;
	    }
	    os << " 0x" << setw(8) << setfill('0') << entry << ",";
	    if (state % 3 == 2)
		  os << endl;
      }
      os << "};" << dec << setfill(' ') << endl;

      os << "static void " << mangle(gate->name()) <<
	    "_output_fun(vvm_simulation*, vvm_bit_t);" << endl;

      os << "static vvm_udp_ssequ<" << gate->pin_count()-1 << "> " <<
	    mangle(gate->name()) << "(&" << mangle(gate->name()) <<
	    "_output_fun, V" << gate->get_initial() << ", " <<
	    mangle(gate->name()) << "_table);" << endl;

	/* The UDP output function is much like other logic gates. Use
	   this general method to output the output function. */
      emit_gate_outputfun_(gate);

}

/*
 * The NetConst is a synthetic device created to represent constant
 * values. I represent them in the output as a vvm_bufz object that
 * has its input connected to nothing but is initialized to the
 * desired constant value.
 */
void target_vvm::net_const(ostream&os, const NetConst*gate)
{
      os << "static void " << mangle(gate->name()) <<
	    "_output_fun(vvm_simulation*, vvm_bit_t);" << endl;

      os << "static vvm_bufz " << mangle(gate->name()) << "(&" <<
	    mangle(gate->name()) << "_output_fun);" << endl;

      init_code << "      " << mangle(gate->name()) << ".set(&sim, 1, ";
      switch (gate->value()) {
	  case verinum::V0:
	    init_code << "V0";
	    break;
	  case verinum::V1:
	    init_code << "V1";
	    break;
	  case verinum::Vx:
	    init_code << "Vx";
	    break;
	  case verinum::Vz:
	    init_code << "Vz";
	    break;
      }
      init_code << ");" << endl;


      emit_gate_outputfun_(gate);
}

void target_vvm::net_esignal(ostream&os, const NetESignal*net)
{
      os << "static vvm_bitset_t<" << net->pin_count() << "> " <<
	    mangle(net->name()) << "_bits; /* " << net->name() <<
	    " */" << endl;
      os << "static vvm_signal_t<" << net->pin_count() << "> " <<
	    mangle(net->name()) << "(\"" << net->name() << "\", &" <<
	    mangle(net->name()) << "_bits);" << endl;
}

/*
 * The net_pevent device is a synthetic device type--a fabrication of
 * the elaboration phase. An event device receives value changes from
 * the attached signal. It is an input only device, its only value
 * being the side-effects that threads waiting on events can be
 * awakened.
 *
 * The proc_event method handles the other half of this, the process
 * that blocks on the event.
 */
void target_vvm::net_pevent(ostream&os, const NetPEvent*gate)
{
      os << "static vvm_pevent " << mangle(gate->name()) << ";"
	    " /* " << gate->name() << " */" << endl;
}

void target_vvm::start_process(ostream&os, const NetProcTop*proc)
{
      process_counter += 1;
      thread_step_ = 0;

      os << "class thread" << process_counter <<
	    "_t : public vvm_thread {" << endl;

      os << "    public:" << endl;
      os << "      thread" << process_counter <<
	    "_t(vvm_simulation*sim)" << endl;
      os << "      : vvm_thread(sim), step_(&step_0_)" << endl;
      os << "      { }" << endl;
      os << "      ~thread" << process_counter << "_t() { }" << endl;
      os << endl;
      os << "      bool go() { return (this->*step_)(); }" << endl;
      os << "    private:" << endl;
      os << "      bool (thread" << process_counter <<
	    "_t::*step_)();" << endl;

      os << "      bool step_0_() {" << endl;
}

/*
 * This method generates code for a procedural assignment. The lval is
 * a signal, but the assignment should generate code to go to all the
 * connected devices/events.
 */
void target_vvm::proc_assign(ostream&os, const NetAssign*net)
{
      string rval = emit_proc_rval(os, 8, net->rval());

      const NetNet*lval;
      unsigned msb, lsb;
      net->find_lval_range(lval, msb, lsb);

      if ((lsb == 0) && (msb == (lval->pin_count()-1))) {
	    os << "        // " << net->get_line() << ": "
	       << lval->name() << " = ";
	    net->rval()->dump(os);
	    os << endl;
      } else {
	    assert(0);
      }

	/* Not only is the lvalue signal assigned to, send the bits to
	   all the other pins that are connected to this signal. */

      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    const NetObj*cur;
	    unsigned pin;
	    for (net->pin(idx).next_link(cur, pin)
		       ; net->pin(idx) != cur->pin(pin)
		       ; cur->pin(pin).next_link(cur, pin)) {

		    // Skip NetAssign nodes. They are output-only.
		  if (dynamic_cast<const NetAssign*>(cur))
			continue;

		    // Skip signals, I'll hit them when I handle the
		    // NetESignal nodes.
		  if (dynamic_cast<const NetNet*>(cur))
			continue;

		  os << "        " << mangle(cur->name()) <<
			".set(sim_, " << pin << ", " <<
			rval << "[" << idx << "]);" << endl;
	    }
      }
}

void target_vvm::proc_block(ostream&os, const NetBlock*net)
{
      net->emit_recurse(os, this);
}

/*
 * The code for a case statement introduces basic blocks so causes
 * steps to be created. There is a step for each case, and the
 * out. For example:
 *
 *    case (foo)
 *        1 : X;
 *        2 : Y;
 *    endcase
 *    Z;
 *
 * causes code for Z to be generated, and also code for X and Y that
 * each branch to Z when they finish. X, Y and Z all generate at least
 * one step.
 */
void target_vvm::proc_case(ostream&os, const NetCase*net)
{
      os << "        /* case (" << *net->expr() << ") */" << endl;
      string expr = emit_proc_rval(os, 8, net->expr());

      ostrstream sc;
      unsigned exit_step = thread_step_ + 1;
      thread_step_ += 1;

      unsigned default_idx = net->nitems();

	/* Handle the case statement like a computed goto, where the
	   result of the case statements is the next state to go
	   to. Once that is done, return true so that statement is
	   executed. */
      for (unsigned idx = 0 ;  idx < net->nitems() ;  idx += 1) {
	    if (net->expr(idx) == 0) {
		  assert(default_idx == net->nitems());
		  default_idx = idx;
		  continue;
	    }
	    assert(net->expr(idx));
	    os << "        /* " << *net->expr(idx) << " : */" << endl;
	    string guard = emit_proc_rval(os, 8, net->expr(idx));

	    thread_step_ += 1;

	    os << "        if (" << expr << ".eequal(" << guard <<
		  ")) {" << endl;
	    os << "            step_ = &step_" <<
		  thread_step_ << "_;" << endl;
	    os << "            return true;" << endl;
	    os << "        }" << endl;

	    sc << "      bool step_" << thread_step_ << "_()" << endl;
	    sc << "      {" << endl;
	    net->stat(idx)->emit_proc(sc, this);
	    sc << "        step_ = &step_" << exit_step << "_;" << endl;
	    sc << "        return true;" << endl;
	    sc << "      }" << endl;
      }

      if (default_idx < net->nitems()) {
	    thread_step_ += 1;

	    os << "        /* default : */" << endl;
	    os << "        step_ = &step_" << thread_step_ << "_;" << endl;

	    sc << "      bool step_" << thread_step_ << "_()" << endl;
	    sc << "      {" << endl;
	    net->stat(default_idx)->emit_proc(sc, this);
	    sc << "        step_ = &step_" << exit_step << "_;" << endl;
	    sc << "        return true;" << endl;
	    sc << "      }" << endl;

      } else {
	    os << "        /* no default ... fall out of case. */" << endl;
	    os << "        step_ = &step_" << exit_step << "_;" << endl;
      }

      os << "        /* endcase */" << endl;
      os << "        return true;" << endl;
      os << "      }" << endl;
      os << sc.str();
      os << "      bool step_" << exit_step << "_()" << endl;
      os << "      {" << endl;
}

void target_vvm::proc_condit(ostream&os, const NetCondit*net)
{
      string expr = emit_proc_rval(os, 8, net->expr());

      unsigned if_step   = ++thread_step_;
      unsigned else_step = ++thread_step_;
      unsigned out_step  = ++thread_step_;

      os << "        if (" << expr << "[0] == V1)" << endl;
      os << "          step_ = &step_" << if_step << "_;" << endl;
      os << "        else" << endl;
      os << "          step_ = &step_" << else_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      };" << endl;

      os << "      bool step_" << if_step << "_()" << endl;
      os << "      {" << endl;
      net->emit_recurse_if(os, this);
      os << "        step_ = &step_" << out_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      }" << endl;

      os << "      bool step_" << else_step << "_()" << endl;
      os << "      {" << endl;
      net->emit_recurse_else(os, this);
      os << "        step_ = &step_" << out_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      }" << endl;

      os << "      bool step_" << out_step << "_()" << endl;
      os << "      {" << endl;
}

void target_vvm::proc_task(ostream&os, const NetTask*net)
{
      if (net->name()[0] == '$') {
	    string ptmp = make_temp();
	    os << "        struct vvm_calltf_parm " << ptmp << "[" <<
		  net->nparms() << "];" << endl;
	    for (unsigned idx = 0 ;  idx < net->nparms() ;  idx += 1)
		  if (net->parm(idx)) {
			string val = emit_parm_rval(os, net->parm(idx));
			os << "        " << ptmp << "[" << idx << "] = " <<
			      val << ";" << endl;
		  }
	    os << "        vvm_calltask(sim_, \"" << net->name() << "\", " <<
		  net->nparms() << ", " << ptmp << ");" << endl;
      } else {
	    os << "        // Huh? " << net->name() << endl;
      }
}

void target_vvm::proc_while(ostream&os, const NetWhile*net)
{
      os << "        for (;;) {" << endl;
      string expr = emit_proc_rval(os, 12, net->expr());
      os << "            if (" << expr << "[0] != V1) break;" << endl;
      net->emit_proc_recurse(os, this);
      os << "        }" << endl;
}

/*
 * Within a process, the proc_event is a statement that is blocked
 * until the event is signalled.
 */
void target_vvm::proc_event(ostream&os, const NetPEvent*proc)
{
      thread_step_ += 1;
      os << "        step_ = &step_" << thread_step_ << "_;" << endl;

	/* POSITIVE is for the wait construct, and needs to be handled
	   specially. The structure of the generated code is:

	     if (event.get()==V1) {
	         return true;
	     } else {
	         event.wait(vvm_pevent::POSEDGE, this);
		 return false;
	     }

	   This causes the wait to not even block the thread if the
	   event value is already positive, otherwise wait for a
	   rising edge. All the edge triggers look like this:

	     event.wait(vvm_pevent::POSEDGE, this);
	     return false;

	   POSEDGE is replaced with the correct type for the desired
	   edge. */

      if (proc->edge() == NetPEvent::POSITIVE) {
	    os << "        if (" << mangle(proc->name()) <<
		  ".get()==V1) {" << endl;
	    os << "           return true;" << endl;
	    os << "        } else {" << endl;
	    os << "           " << mangle(proc->name()) <<
		  ".wait(vvm_pevent::POSEDGE, this);" << endl;
	    os << "           return false;" << endl;
	    os << "        }" << endl;

      } else {
	    os << "        " << mangle(proc->name()) << ".wait(vvm_pevent::";
	    switch (proc->edge()) {
		case NetPEvent::ANYEDGE:
		  os << "ANYEDGE";
		  break;
		case NetPEvent::POSITIVE:
		case NetPEvent::POSEDGE:
		  os << "POSEDGE";
		  break;
		case NetPEvent::NEGEDGE:
		  os << "NEGEDGE";
		  break;
	    }
	    os << ", this);" << endl;
	    os << "           return false;" << endl;
      }
      os << "      }" << endl;
      os << "      bool step_" << thread_step_ << "_()" << endl;
      os << "      {" << endl;

      proc->emit_proc_recurse(os, this);
}

/*
 * A delay suspends the thread for a period of time.
 */
void target_vvm::proc_delay(ostream&os, const NetPDelay*proc)
{
      thread_step_ += 1;
      os << "        step_ = &step_" << thread_step_ << "_;" << endl;
      os << "        sim_->thread_delay(" << proc->delay() << ", this);"
	 << endl;
      os << "        return false;" << endl;
      os << "      }" << endl;
      os << "      bool step_" << thread_step_ << "_()" << endl;
      os << "      {" << endl;

      proc->emit_proc_recurse(os, this);
}

void target_vvm::end_process(ostream&os, const NetProcTop*proc)
{
      if (proc->type() == NetProcTop::KALWAYS) {
	    os << "        step_ = &step_0_;" << endl;
	    os << "        return true;" << endl;
      } else {
	    os << "        step_ = 0;" << endl;
	    os << "        return false;" << endl;
      }

      os << "      }" << endl;
      os << "};" << endl;
}


static target_vvm target_vvm_obj;

extern const struct target tgt_vvm = {
      "vvm",
      &target_vvm_obj
};
/*
 * $Log: t-vvm.cc,v $
 * Revision 1.14  1999/03/15 02:43:32  steve
 *  Support more operators, especially logical.
 *
 * Revision 1.13  1999/02/22 03:01:12  steve
 *  Handle default case.
 *
 * Revision 1.12  1999/02/15 02:06:15  steve
 *  Elaborate gate ranges.
 *
 * Revision 1.11  1999/02/08 03:55:55  steve
 *  Do not generate code for signals,
 *  instead use the NetESignal node to
 *  generate gate-like signal devices.
 *
 * Revision 1.10  1999/02/08 02:49:56  steve
 *  Turn the NetESignal into a NetNode so
 *  that it can connect to the netlist.
 *  Implement the case statement.
 *  Convince t-vvm to output code for
 *  the case statement.
 *
 * Revision 1.9  1999/01/01 01:46:01  steve
 *  Add startup after initialization.
 *
 * Revision 1.8  1998/12/20 02:05:41  steve
 *  Function to calculate wire initial value.
 *
 * Revision 1.7  1998/12/17 23:54:58  steve
 *  VVM support for small sequential UDP objects.
 *
 * Revision 1.6  1998/11/23 00:20:23  steve
 *  NetAssign handles lvalues as pin links
 *  instead of a signal pointer,
 *  Wire attributes added,
 *  Ability to parse UDP descriptions added,
 *  XNF generates EXT records for signals with
 *  the PAD attribute.
 *
 * Revision 1.5  1998/11/10 00:48:31  steve
 *  Add support it vvm target for level-sensitive
 *  triggers (i.e. the Verilog wait).
 *  Fix display of $time is format strings.
 *
 * Revision 1.4  1998/11/09 18:55:34  steve
 *  Add procedural while loops,
 *  Parse procedural for loops,
 *  Add procedural wait statements,
 *  Add constant nodes,
 *  Add XNOR logic gate,
 *  Make vvm output look a bit prettier.
 *
 * Revision 1.3  1998/11/07 19:17:10  steve
 *  Calculate expression widths at elaboration time.
 *
 * Revision 1.2  1998/11/07 17:05:06  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:29:05  steve
 *  Introduce verilog to CVS.
 *
 */

