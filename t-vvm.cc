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
#if !defined(WINNT)
#ident "$Id: t-vvm.cc,v 1.47 1999/09/28 01:21:27 steve Exp $"
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
      virtual void memory(ostream&os, const NetMemory*);
      virtual void task_def(ostream&os, const NetTaskDef*);
      virtual void func_def(ostream&os, const NetFuncDef*);

      virtual void lpm_add_sub(ostream&os, const NetAddSub*);

      virtual void logic(ostream&os, const NetLogic*);
      virtual void bufz(ostream&os, const NetBUFZ*);
      virtual void udp(ostream&os, const NetUDP*);
      virtual void net_assign_nb(ostream&os, const NetAssignNB*);
      virtual void net_const(ostream&os, const NetConst*);
      virtual void net_esignal(ostream&os, const NetESignal*);
      virtual void net_event(ostream&os, const NetNEvent*);
      virtual bool process(ostream&os, const NetProcTop*);
      virtual void proc_assign(ostream&os, const NetAssign*);
      virtual void proc_assign_mem(ostream&os, const NetAssignMem*);
      virtual void proc_assign_nb(ostream&os, const NetAssignNB*);
      virtual bool proc_block(ostream&os, const NetBlock*);
      virtual void proc_case(ostream&os, const NetCase*net);
      virtual void proc_condit(ostream&os, const NetCondit*);
      virtual void proc_forever(ostream&os, const NetForever*);
      virtual void proc_repeat(ostream&os, const NetRepeat*);
      virtual void proc_stask(ostream&os, const NetSTask*);
      virtual void proc_utask(ostream&os, const NetUTask*);
      virtual void proc_while(ostream&os, const NetWhile*);
      virtual void proc_event(ostream&os, const NetPEvent*);
      virtual void proc_delay(ostream&os, const NetPDelay*);
      virtual void end_design(ostream&os, const Design*);

      void start_process(ostream&os, const NetProcTop*);
      void end_process(ostream&os, const NetProcTop*);

    private:
      void emit_gate_outputfun_(const NetNode*);

      ostrstream delayed;
      ostrstream init_code;
      ostrstream start_code;
      unsigned process_counter;
      unsigned thread_step_;

	// These methods are use to help prefent duplicate printouts
	// of things that may be scanned multiple times.
      map<string,bool>esignal_printed_flag;
      map<string,bool>pevent_printed_flag;
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
      virtual void expr_concat(const NetEConcat*);
      virtual void expr_ident(const NetEIdent*);
      virtual void expr_memory(const NetEMemory*mem)
	    {
		  mem->index()->expr_scan(this);
		  string idx = make_temp();
		  os_ << setw(indent_) << "" << "const unsigned " <<
			idx << " = " << result << ".as_unsigned();" <<
			endl;
		  result = mangle(mem->name()) + "[" + idx + "]";
	    }
      virtual void expr_signal(const NetESignal*);
      virtual void expr_subsignal(const NetESubSignal*sig);
      virtual void expr_ternary(const NetETernary*);
      virtual void expr_unary(const NetEUnary*);
      virtual void expr_binary(const NetEBinary*);
      virtual void expr_ufunc(const NetEUFunc*);
};

void vvm_proc_rval::expr_concat(const NetEConcat*expr)
{
      assert(expr->repeat() == 1);
      string tname = make_temp();
      os_ << setw(indent_) << "" << "vvm_bitset_t<" <<
	    expr->expr_width() << "> " << tname << ";" << endl;

      unsigned pos = 0;
      for (unsigned idx = 0 ;  idx < expr->nparms() ;  idx += 1) {

	    NetExpr*pp = expr->parm(expr->nparms() - idx - 1);
	    pp->expr_scan(this);

	    for (unsigned bit = 0 ;  bit < pp->expr_width() ;  bit += 1) {
		  os_ << setw(indent_) << "" << tname << "[" << pos <<
			"] = " << result << "[" << bit << "];" <<
			endl;
		  pos+= 1;
	    }
	    assert(pos <= expr->expr_width());
      }
      assert(pos == expr->expr_width());

      result = tname;
}

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

void vvm_proc_rval::expr_subsignal(const NetESubSignal*sig)
{
      string idx = make_temp();
      string val = make_temp();
      if (const NetEConst*cp = dynamic_cast<const NetEConst*>(sig->index())) {
	    os_ << setw(indent_) << "" << "const unsigned " << idx <<
		  " = " << cp->value().as_ulong() << ";" << endl;

      } else {
	    sig->index()->expr_scan(this);
	    os_ << setw(indent_) << "" << "const unsigned " <<
		  idx << " = " << result << ".as_unsigned();" <<
		  endl;
      }

      os_ << setw(indent_) << "" << "vvm_bitset_t<1>" << val << ";" << endl;
      os_ << setw(indent_) << "" << val << "[0] = " <<
	    mangle(sig->name()) << "_bits[" << idx << "];" << endl;
      result = val;
}

void vvm_proc_rval::expr_ternary(const NetETernary*expr)
{
      expr->cond_expr()->expr_scan(this);
      string cond_val = result;
      expr->true_expr()->expr_scan(this);
      string true_val = result;
      expr->false_expr()->expr_scan(this);
      string false_val = result;

      result = make_temp();

      os_ << setw(indent_) << "" << "vvm_bitset_t<" <<
	    expr->expr_width() << ">" << result << ";" << endl;
      os_ << setw(indent_) << "" << result << " = vvm_ternary(" <<
	    cond_val << "[0], " << true_val << ", " << false_val << ");"
	  << endl;
}

/*
 * A function call is handled by assigning the parameters from the
 * input expressions, then calling the function. After the function
 * returns, copy the result into a temporary variable.
 *
 * Function calls are different from tasks in this regard--tasks had
 * all this assigning arranged during elaboration. For functions, we
 * must do it ourselves.
 */
void vvm_proc_rval::expr_ufunc(const NetEUFunc*expr)
{
      const NetFuncDef*def = expr->definition();
      const unsigned pcnt = expr->parm_count();
      assert(pcnt == (def->port_count()-1));

	/* Scan the parameter expressions, and assign the values to
	   the parameter port register. */
      for (unsigned idx = 0 ;  idx < pcnt ;  idx += 1) {
	    expr->parm(idx)->expr_scan(this);
	    os_ << "        " << mangle(def->port(idx+1)->name()) <<
		  "_bits = " << result << ";" << endl;
      }

	/* Make the function call. */
      os_ << "        " << mangle(expr->name()) << "(sim_);" << endl;

	/* Save the return value in a temporary. */
      result = make_temp();
      string rbits = mangle(expr->result()->name()) + "_bits";

      os_ << "        vvm_bitset_t<" << expr->expr_width() << "> " <<
	    result << " = " << rbits << ";" << endl;
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
	  case '&':
	    os_ << "vvm_unop_and(" << result << ");"
		<< endl;
	    break;
	  case '!':
	    os_ << "vvm_unop_lnot(" << result << ");"
		<< endl;
	    break;
	  default:
	    cerr << "vvm: Unhandled unary op `" << expr->op() << "'"
		 << endl;
	    os_ << "#error \"" << expr->get_line() << ": vvm error: "
		  "Unhandled unary op: " << *expr << "\"" << endl;
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
	  case 'a': // logical and (&&)
	    os_ << setw(indent_) << "" << result << " = vvm_binop_land("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'E': // ===
	    os_ << setw(indent_) << "" << result << " = vvm_binop_eeq("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'e': // ==
	    os_ << setw(indent_) << "" << result << " = vvm_binop_eq("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'G': // >=
	    os_ << setw(indent_) << "" << result << " = vvm_binop_ge("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'l': // left shift(<<)
	    os_ << setw(indent_) << "" << result << " = vvm_binop_shiftl("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'L': // <=
	    os_ << setw(indent_) << "" << result << " = vvm_binop_le("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'N': // !==
	    os_ << setw(indent_) << "" << result << " = vvm_binop_nee("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'n':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_ne("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case '<':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_lt("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case '>':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_gt("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'o': // logical or (||)
	    os_ << setw(indent_) << "" << result << " = vvm_binop_lor("
		<< lres << "," << rres << ");" << endl;
	  case 'r': // right shift(>>)
	    os_ << setw(indent_) << "" << result << " = vvm_binop_shiftr("
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
	  case '^':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_xor("
		<< lres << "," << rres << ");" << endl;
	    break;
	  default:
	    cerr << "vvm: Unhandled binary op `" << expr->op() << "': "
		 << *expr << endl;
	    os_ << "#error \"" << expr->get_line() << ": vvm error: "
		  "Unhandled binary op: " << *expr << "\"" << endl;
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

/*
 * The vvm_parm_rval class scans expressions for the purpose of making
 * parameters for system tasks/functions. Thus, the generated code is
 * geared towards making the handles needed to make the call.
 *
 * The result of any parm rval scan is a vpiHandle, or a string that
 * automatically converts to a vpiHandle on assignment.
 */
class vvm_parm_rval  : public expr_scan_t {

    public:
      explicit vvm_parm_rval(ostream&os)
      : result(""), os_(os) { }

      string result;

    private:
      virtual void expr_const(const NetEConst*expr);
      virtual void expr_ident(const NetEIdent*);
      virtual void expr_signal(const NetESignal*);

    private:
      ostream&os_;
};

void vvm_parm_rval::expr_const(const NetEConst*expr)
{
      if (expr->value().is_string()) {
	    result = make_temp();
	    os_ << "        struct __vpiHandle " << result << ";" << endl;
	    os_ << "        vvm_make_vpi_parm(&" << result << ", \""
		<< expr->value().as_string() << "\");" << endl;
	    result = "&" + result;
	    return;
      }

      string tname = make_temp();
      os_ << "        vvm_bitset_t<" <<
	    expr->expr_width() << "> " << tname << ";" << endl;
      for (unsigned idx = 0 ;  idx < expr->expr_width() ;  idx += 1) {
	    os_ << "        " << tname << "[" << idx << "] = ";
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

      result = make_temp();
      os_ << "        struct __vpiHandle " << result << ";" << endl;
      os_ << "        vvm_make_vpi_parm(&" << result << ", &" << tname
	  << ");" << endl;
      result = "&" + result;
}

void vvm_parm_rval::expr_ident(const NetEIdent*expr)
{
      if (expr->name() == "$time") {
	    os_ << "        system_time.val.time.low "
		  "= sim_->get_sim_time();" << endl;
	    result = string("&system_time");
      } else {
	    cerr << "Unhandled identifier: " << expr->name() << endl;
      }
}

void vvm_parm_rval::expr_signal(const NetESignal*expr)
{
      string res = string("&") + mangle(expr->name()) + "_vpi";
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
      os << "# include \"vpi_user.h\"" << endl;
      os << "# include \"vpi_priv.h\"" << endl;

      os << "static struct __vpiHandle system_time;" << endl;
      process_counter = 0;

      init_code << "static void design_init(vvm_simulation&sim)" << endl;
      init_code << "{" << endl;
      init_code << "      vvm_init_vpi_timevar(&system_time, \"$time\");"
		<< endl;
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
      os << "      vvm_load_vpi_module(\"system.vpi\");" << endl;
      os << "      vvm_simulation sim;" << endl;
      os << "      design_init(sim);" << endl;
      os << "      design_start(sim);" << endl;

      for (unsigned idx = 0 ;  idx < process_counter ;  idx += 1)
	    os << "      thread" << (idx+1) << "_t thread_" <<
		  (idx+1) << "(&sim);" << endl;

      os << "      sim.run();" << endl;
      os << "}" << endl;
}

bool target_vvm::process(ostream&os, const NetProcTop*top)
{
      start_process(os, top);
      bool rc = top->statement()->emit_proc(os, this);
      end_process(os, top);
      return rc;
}

void target_vvm::signal(ostream&os, const NetNet*sig)
{

	/* Scan the signals of the vector, passing the initial value
	   to the inputs of all the connected devices. */
      for (unsigned idx = 0 ;  idx < sig->pin_count() ;  idx += 1) {
	    if (sig->get_ival(idx) == verinum::Vz)
		  continue;

	    map<string,bool>written;

	    for (const NetObj::Link*lnk = sig->pin(idx).next_link()
		       ; (*lnk) != sig->pin(idx) ;  lnk = lnk->next_link()) {

		  if (lnk->get_dir() == NetObj::Link::OUTPUT)
			continue;

		    // Check to see if the name has already been
		    // written to. This can happen if the object is a
		    // NetESignal, because there can be many of them
		    // with the same name.
		  if (written[lnk->get_obj()->name()])
			continue;

		  written[lnk->get_obj()->name()] = true;


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

void target_vvm::memory(ostream&os, const NetMemory*mem)
{
      os << "static vvm_bitset_t<" << mem->width() << "> " <<
	    mangle(mem->name()) << "[" << mem->count() << "]; " <<
	    "/* " << mem->name() << " */" << endl;
}

void target_vvm::task_def(ostream&os, const NetTaskDef*def)
{
      thread_step_ = 0;
      const string name = mangle(def->name());
      os << "class " << name << "  : public vvm_thread {" << endl;
      os << "    public:" << endl;
      os << "      " << name << "(vvm_simulation*sim, vvm_thread*th)" << endl;
      os << "      : vvm_thread(sim), back_(th), step_(&step_0_)" << endl;
      os << "      { }" << endl;
      os << "      ~" << name << "() { }" << endl;
      os << "      bool go() { return (this->*step_)(); }" << endl;
      os << "    private:" << endl;
      os << "      vvm_thread*back_;" << endl;
      os << "      bool (" << name << "::*step_)();" << endl;
      os << "      bool step_0_() {" << endl;
      def->proc()->emit_proc(os, this);
      os << "        sim_->thread_active(back_);" << endl;
      os << "        return false;" << endl;
      os << "      }" << endl;
      os << "};" << endl;
}

/*
 * A function definition is emitted as a C++ function that takes no
 * parameters and returns no result. The actual parameter passing
 * happens in the function call, where the signals that are the inputs
 * are assigned by the caller, the caller calls the function (which
 * writes the result) then the caller copies the result out of the
 * magic result register.
 */
void target_vvm::func_def(ostream&os, const NetFuncDef*def)
{
      thread_step_ = 0;
      const string name = mangle(def->name());
      os << "// Function " << def->name() << endl;
      os << "static void " << name << "(vvm_simulation*);" << endl;

      delayed << "// Function " << def->name() << endl;
      delayed << "static void " << name << "(vvm_simulation*sim_)" << endl;
      delayed << "{" << endl;
      def->proc()->emit_proc(delayed, this);
      delayed << "}" << endl;
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

void target_vvm::lpm_add_sub(ostream&os, const NetAddSub*gate)
{
      os << "#error \"adders not yet supported in vvm.\"" << endl;
      os << "static vvm_add_sub<" << gate->width() << "> " <<
	    mangle(gate->name()) << ";" << endl;
}

void target_vvm::logic(ostream&os, const NetLogic*gate)
{
      os << "static void " << mangle(gate->name()) <<
	    "_output_fun(vvm_simulation*, vvm_bit_t);" << endl;

      switch (gate->type()) {
	  case NetLogic::AND:
	    os << "static vvm_and" << "<" << gate->pin_count()-1 <<
		  "," << gate->rise_time() << "> ";
	    break;
	  case NetLogic::BUFIF0:
	    os << "static vvm_bufif0<" << gate->rise_time() << "> ";
	    break;
	  case NetLogic::BUFIF1:
	    os << "static vvm_bufif1<" << gate->rise_time() << "> ";
	    break;
	  case NetLogic::NAND:
	    os << "static vvm_nand" << "<" << gate->pin_count()-1 <<
		  "," << gate->rise_time() << "> ";
	    break;
	  case NetLogic::NOR:
	    os << "static vvm_nor" << "<" << gate->pin_count()-1 <<
		  "," << gate->rise_time() << "> ";
	    break;
	  case NetLogic::NOT:
	    os << "static vvm_not" << "<" << gate->rise_time() << "> ";
	    break;
	  case NetLogic::OR:
	    os << "static vvm_or" << "<" << gate->pin_count()-1 <<
		  "," << gate->rise_time() << "> ";
	    break;
	  case NetLogic::XNOR:
	    os << "static vvm_xnor" << "<" << gate->pin_count()-1 <<
		  "," << gate->rise_time() << "> ";
	    break;
	  case NetLogic::XOR:
	    os << "static vvm_xor" << "<" << gate->pin_count()-1 <<
		  "," << gate->rise_time() << "> ";
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
 * The non-blocking assignment works by creating an event to do the
 * assignment at the right time. The value to be assigned is saved in
 * the event and the event function performs the actual assignment.
 *
 * The net part of the assign generates a type ot represent the
 * assignment. Creating instances of this event will be dealt with
 * later.
 */
void target_vvm::net_assign_nb(ostream&os, const NetAssignNB*net)
{
      const string name = mangle(net->name());
      unsigned iwid = net->rval()->expr_width();
      os << "class " << name << " : public vvm_event {" << endl;
      os << "    public:" << endl;

      if (net->bmux()) {
	    os << "      " << name << "(vvm_simulation*s, const vvm_bitset_t<"
	       << iwid << ">&v, unsigned idx)" << endl;
	    os << "      : sim_(s), value_(v), idx_(idx) { }" << endl;
      } else {
	    os << "      " << name << "(vvm_simulation*s, const vvm_bitset_t<"
	       << iwid << ">&v)" << endl;
	    os << "      : sim_(s), value_(v) { }" << endl;
      }
      os << "      void event_function();" << endl;

      os << "    private:" << endl;
      os << "      vvm_simulation*sim_;" << endl;
      os << "      vvm_bitset_t<" << iwid << ">value_;" << endl;

      if (net->bmux())
	    os << "      unsigned idx_;" << endl;

      os << "};" << endl;


	/* Write the event_function to do the actual assignment. */

      delayed << "void " << name << "::event_function()" << endl;
      delayed << "{" << endl;

      if (net->bmux()) {
	      /* If the assignment is to a single bit (with a mux)
		 then write a switch statement that selects which pins
		 to write to. */
	    delayed << "      switch (idx_) {" << endl;
	    for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
		  const NetObj*cur;
		  unsigned pin;

		  delayed << "        case " << idx << ":" << endl;
		  for (net->pin(idx).next_link(cur, pin)
			     ; net->pin(idx) != cur->pin(pin)
			     ; cur->pin(pin).next_link(cur, pin)) {

			  // Skip output only pins.
			if (cur->pin(pin).get_dir() == NetObj::Link::OUTPUT)
			      continue;

			  // Skip signals, I'll hit them when I handle the
			  // NetESignal nodes.
			if (dynamic_cast<const NetNet*>(cur))
			      continue;

			delayed << "          " << mangle(cur->name()) <<
			      ".set(sim_, " << pin << ", value_[0]);" << endl;
		  }

		  delayed << "          break;" << endl;
	    }
	    delayed << "      }" << endl;

      } else {

	      /* If there is no BMUX, then write all the bits of the
		 value to all the pins. */
	    for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
		  const NetObj*cur;
		  unsigned pin;

		  for (net->pin(idx).next_link(cur, pin)
			     ; net->pin(idx) != cur->pin(pin)
			     ; cur->pin(pin).next_link(cur, pin)) {

			  // Skip output only pins.
			if (cur->pin(pin).get_dir() == NetObj::Link::OUTPUT)
			      continue;

			  // Skip signals, I'll hit them when I handle the
			  // NetESignal nodes.
			if (dynamic_cast<const NetNet*>(cur))
			      continue;

			delayed << "      " << mangle(cur->name()) <<
			      ".set(sim_, " << pin << ", value_[" <<
			      idx << "]);" << endl;
		  }
	    }
      }
      delayed << "}" << endl;
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
      bool&flag = esignal_printed_flag[net->name()];
      if (flag)
	    return;

      flag = true;
      os << "static vvm_bitset_t<" << net->pin_count() << "> " <<
	    mangle(net->name()) << "_bits; /* " << net->name() <<
	    " */" << endl;
      os << "static vvm_signal_t<" << net->pin_count() << "> " <<
	    mangle(net->name()) << "(\"" << net->name() << "\", &" <<
	    mangle(net->name()) << "_bits);" << endl;

      os << "static struct __vpiHandle " << mangle(net->name()) <<
	    "_vpi;" << endl;

      init_code << "      vvm_init_vpi_handle(&" <<
	    mangle(net->name()) << "_vpi, &" << mangle(net->name()) <<
	    "_bits, &" << mangle(net->name()) << ");" << endl;
      
}

/*
 * The net_event device is a synthetic device type--a fabrication of
 * the elaboration phase. An event device receives value changes from
 * the attached signal. It is an input only device, its only value
 * being the side-effects that threads waiting on events can be
 * awakened.
 *
 * The proc_event method handles the other half of this, the process
 * that blocks on the event.
 */
void target_vvm::net_event(ostream&os, const NetNEvent*gate)
{
      string pevent = mangle(gate->fore_ptr()->name());
      os << "  /* " << gate->name() << " */" << endl;

      bool&printed = pevent_printed_flag[pevent];
      if (! printed) {
	    printed = true;
	    os << "static vvm_sync " << pevent << ";" << endl;
      }

      os << "static vvm_pevent<" << gate->pin_count() << "> " <<
	    mangle(gate->name()) << "(&" << pevent << ", ";
      switch (gate->type()) {
	  case NetNEvent::POSEDGE:
	  case NetNEvent::POSITIVE:
	    os << "vvm_pevent<" << gate->pin_count() << ">::POSEDGE";
	    break;
	  case NetNEvent::NEGEDGE:
	    os << "vvm_pevent<"<<  gate->pin_count() << ">::NEGEDGE";
	    break;
	  case NetNEvent::ANYEDGE:
	    os << "vvm_pevent<" << gate->pin_count() << ">::ANYEDGE";
	    break;
      }
      os << ");" << endl;
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
      os << "      : vvm_thread(sim), step_(&thread" <<
	    process_counter << "_t::step_0_)" << endl;
      os << "      { }" << endl;
      os << "      ~thread" << process_counter << "_t() { }" << endl;
      os << endl;
      os << "      bool go() { return (this->*step_)(); }" << endl;
      os << "    private:" << endl;
      os << "      bool (thread" << process_counter <<
	    "_t::*step_)();" << endl;
      os << "      vvm_thread*callee_;" << endl;
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

      os << "        // " << net->get_line() << ": " << endl;

	/* Not only is the lvalue signal assigned to, send the bits to
	   all the other pins that are connected to this signal. */

      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    const NetObj*cur;
	    unsigned pin;
	    map<string,bool> written;

	    for (net->pin(idx).next_link(cur, pin)
		       ; net->pin(idx) != cur->pin(pin)
		       ; cur->pin(pin).next_link(cur, pin)) {

		    // Skip output only pins.
		  if (cur->pin(pin).get_dir() == NetObj::Link::OUTPUT)
			continue;

		    // Skip signals, I'll hit them when I handle the
		    // NetESignal nodes.
		  if (dynamic_cast<const NetNet*>(cur))
			continue;

		    // It is possible for a named device to show up
		    // several times in a link. This is the classic
		    // case with NetESignal objects, which are
		    // repeated for each expression that uses it.
		  if (written[cur->name()])
			continue;

		  written[cur->name()] = true;
		  os << "        " << mangle(cur->name()) <<
			".set(sim_, " << pin << ", " <<
			rval << "[" << idx << "]);" << endl;
	    }
      }
}

void target_vvm::proc_assign_mem(ostream&os, const NetAssignMem*amem)
{
      string index = emit_proc_rval(os, 8, amem->index());
      string rval = emit_proc_rval(os, 8, amem->rval());
      const NetMemory*mem = amem->memory();

      os << "        /* " << amem->get_line() << " */" << endl;
      os << "        " << mangle(mem->name())
	 << "[" << index << ".as_unsigned()] = " << rval << ";" << endl;
}

void target_vvm::proc_assign_nb(ostream&os, const NetAssignNB*net)
{
      string rval = emit_proc_rval(os, 8, net->rval());

      if (net->bmux()) {
	    string bval = emit_proc_rval(os, 8, net->bmux());
	    os << "        sim_->insert_event(" << net->rise_time()
	       << ", new " << mangle(net->name()) << "(sim_, " << rval
	       << ", " << bval << ".as_unsigned()));" << endl;

      } else {
	    os << "        sim_->insert_event(" << net->rise_time()
	       << ", new " << mangle(net->name()) << "(sim_, " << rval
	       << "));" << endl;
      }
}

bool target_vvm::proc_block(ostream&os, const NetBlock*net)
{
      if (net->type() == NetBlock::PARA) {
	    cerr << "sorry: vvm cannot emit parallel blocks." << endl;
	    return false;
      }
      net->emit_recurse(os, this);
      return true;
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
	    os << "            step_ = &thread" << process_counter <<
		  "_t::step_" << thread_step_ << "_;" << endl;
	    os << "            return true;" << endl;
	    os << "        }" << endl;

	    sc << "      bool step_" << thread_step_ << "_()" << endl;
	    sc << "      {" << endl;
	    if (net->stat(idx))
		  net->stat(idx)->emit_proc(sc, this);
	    sc << "        step_ = &thread" << process_counter <<
		  "_t::step_" << exit_step << "_;" << endl;
	    sc << "        return true;" << endl;
	    sc << "      }" << endl;
      }

      if (default_idx < net->nitems()) {
	    thread_step_ += 1;

	    os << "        /* default : */" << endl;
	    os << "        step_ = &step_" << thread_step_ << "_;" << endl;

	    sc << "      bool step_" << thread_step_ << "_()" << endl;
	    sc << "      {" << endl;
	    if (net->stat(default_idx))
		  net->stat(default_idx)->emit_proc(sc, this);
	    sc << "        step_ = &thread" << process_counter <<
		  "_t::step_" << exit_step << "_;" << endl;
	    sc << "        return true;" << endl;
	    sc << "      }" << endl;

      } else {
	    os << "        /* no default ... fall out of case. */" << endl;
	    os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << exit_step << "_;" << endl;
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
      os << "          step_ = &thread" << process_counter <<
		  "_t::step_" << if_step << "_;" << endl;
      os << "        else" << endl;
      os << "          step_ = &thread" << process_counter <<
		  "_t::step_" << else_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      };" << endl;

      os << "      bool step_" << if_step << "_()" << endl;
      os << "      {" << endl;
      net->emit_recurse_if(os, this);
      os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << out_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      }" << endl;

      os << "      bool step_" << else_step << "_()" << endl;
      os << "      {" << endl;
      net->emit_recurse_else(os, this);
      os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << out_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      }" << endl;

      os << "      bool step_" << out_step << "_()" << endl;
      os << "      {" << endl;
}

/*
 * The forever loop is implemented by starting a basic block, handing
 * the statement, and putting in a goto to the beginning of the block.
 */
void target_vvm::proc_forever(ostream&os, const NetForever*net)
{
      unsigned top_step = ++thread_step_;
      unsigned out_step = ++thread_step_;

      os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << top_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      }" << endl;
      os << "      bool step_" << top_step << "_()" << endl;
      os << "      {" << endl;
      net->emit_recurse(os, this);
      os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << top_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      }" << endl;

      os << "      bool step_" << out_step << "_()" << endl;
      os << "      {" << endl;
}

void target_vvm::proc_repeat(ostream&os, const NetRepeat*net)
{
      string expr = emit_proc_rval(os, 8, net->expr());
      unsigned top_step = ++thread_step_;
      unsigned out_step = ++thread_step_;

      os << "        step_" << top_step << "_idx_ = " << expr <<
	    ".as_unsigned();" << endl;
      os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << top_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      }" << endl;

      os << "      unsigned step_" << top_step << "_idx_;" << endl;

      os << "      bool step_" << top_step << "_()" << endl;
      os << "      {" << endl;
      os << "        if (step_" << top_step << "_idx_ == 0) {" << endl;
      os << "          step_ = &thread" << process_counter <<
		  "_t::step_" << out_step << "_;" << endl;
      os << "          return true;" << endl;
      os << "        }" << endl;
      os << "        step_" << top_step << "_idx_ -= 1;" << endl;

      net->emit_recurse(os,this);

      os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << top_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      }" << endl;

      os << "      bool step_" << out_step << "_()" << endl;
      os << "      {" << endl;
}

/*
 * Calls to system tasks are done here. We know that this is a system
 * task and that I need to generate an external call. Calls to user
 * defined tasks are handled elsewhere.
 */
void target_vvm::proc_stask(ostream&os, const NetSTask*net)
{
      string ptmp = make_temp();

      os << "        vpiHandle " << ptmp << "[" << net->nparms() <<
	    "];" << endl;
      for (unsigned idx = 0 ;  idx < net->nparms() ;  idx += 1) {
	    string val;
	    if (net->parm(idx)) {
		  val = emit_parm_rval(os, net->parm(idx));

	    } else {
		  val = make_temp();
		  os << "        struct __vpiHandle " << val << ";" << endl;
		  os << "        vvm_make_vpi_parm(&" << val << ");" << endl;
		  val = string("&") + val;
	    }

	    os << "        " << ptmp << "[" << idx << "] = " << val << ";"
	       << endl;
      }

      os << "        vvm_calltask(sim_, \"" << net->name() << "\", " <<
	    net->nparms() << ", " << ptmp << ");" << endl;

}

void target_vvm::proc_utask(ostream&os, const NetUTask*net)
{
      unsigned out_step = ++thread_step_;
      const string name = mangle(net->name());
      os << "        callee_ = new " << name << "(sim_, this);" << endl;
      os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << out_step << "_;" << endl;
      os << "        return false;" << endl;
      os << "      }" << endl;
      os << "      bool step_" << out_step << "_()" << endl;
      os << "      {" << endl;
      os << "        delete callee_;" << endl;
}

/*
 * The while loop is implemented by making each iteration one [or
 * more] basic block and letting the loop condition skip to the block
 * after or continue with the current block. This is similar to how
 * the condit is handled. The basic structure of the loop is as follows:
 *
 *    head_step:
 *       evaluate condition
 *       if false, go to out_step
 *       execute body
 *
 *    out_step:
 */
void target_vvm::proc_while(ostream&os, const NetWhile*net)
{
      unsigned head_step = ++thread_step_;
      unsigned out_step = ++thread_step_;

      os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << head_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      }" << endl;

      os << "      // " << net->expr()->get_line() <<
	    ": top of while condition." << endl;
      os << "      bool step_" << head_step << "_()" << endl;
      os << "      {" << endl;

      string expr = emit_proc_rval(os, 8, net->expr());
      os << "        // " << net->expr()->get_line() <<
	    ": test while condition." << endl;
      os << "        if (" << expr << "[0] != V1) {" << endl;
      os << "            step_ = &thread" << process_counter <<
		  "_t::step_" << out_step << "_;" << endl;
      os << "            return true;" << endl;
      os << "        }" << endl;

      net->emit_proc_recurse(os, this);

      os << "        // " << net->expr()->get_line() <<
	    ": end of while loop." << endl;
      os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << head_step << "_;" << endl;
      os << "        return true;" << endl;
      os << "      }" << endl;

      os << "      bool step_" << out_step << "_()" << endl;
      os << "      {" << endl;
}

/*
 * Within a process, the proc_event is a statement that is blocked
 * until the event is signalled.
 */
void target_vvm::proc_event(ostream&os, const NetPEvent*proc)
{
      thread_step_ += 1;
      os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << thread_step_ << "_;" << endl;

	/* POSITIVE is for the wait construct, and needs to be handled
	   specially. The structure of the generated code is:

	     if (event.get()==V1) {
	         return true;
	     } else {
	         event.wait(this);
		 return false;
	     }

	   This causes the wait to not even block the thread if the
	   event value is already positive, otherwise wait for a
	   rising edge. All the edge triggers look like this:

	     event.wait(vvm_pevent::POSEDGE, this);
	     return false;

	   POSEDGE is replaced with the correct type for the desired
	   edge. */

      svector<const NetNEvent*>*list = proc->back_list();
      if ((list->count()==1) && ((*list)[0]->type() == NetNEvent::POSITIVE)) {
	    os << "        if (" << mangle((*list)[0]->name()) <<
		  ".get()[0]==V1) {" << endl;
	    os << "           return true;" << endl;
	    os << "        } else {" << endl;
	    os << "           " << mangle(proc->name()) <<
		  ".wait(this);" << endl;
	    os << "           return false;" << endl;
	    os << "        }" << endl;
      } else {
	      /* The canonical wait for an edge puts the thread into
		 the correct wait object, then returns false from the
		 thread to suspend execution. When things are ready to
		 proceed, the correct vvm_pevent will send a wakeup to
		 start the next basic block. */
	    os << "        " << mangle(proc->name()) << ".wait(this);" << endl;
	    os << "        return false;" << endl;
      }

      os << "      }" << endl;
      os << "      bool step_" << thread_step_ << "_()" << endl;
      os << "      {" << endl;

      proc->emit_proc_recurse(os, this);
      delete list;
}

/*
 * A delay suspends the thread for a period of time.
 */
void target_vvm::proc_delay(ostream&os, const NetPDelay*proc)
{
      thread_step_ += 1;
      os << "        step_ = &thread" << process_counter <<
		  "_t::step_" << thread_step_ << "_;" << endl;
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
	    os << "        step_ = &thread" << process_counter <<
		  "_t::step_0_;" << endl;
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
 * Revision 1.47  1999/09/28 01:21:27  steve
 *  Proper syntax for method pointers.
 *
 * Revision 1.46  1999/09/28 01:13:15  steve
 *  Support in vvm > and >= behavioral operators.
 *
 * Revision 1.45  1999/09/23 03:56:57  steve
 *  Support shift operators.
 *
 * Revision 1.44  1999/09/22 16:57:24  steve
 *  Catch parallel blocks in vvm emit.
 *
 * Revision 1.43  1999/09/22 04:30:04  steve
 *  Parse and elaborate named for/join blocks.
 *
 * Revision 1.42  1999/09/16 04:18:15  steve
 *  elaborate concatenation repeats.
 *
 * Revision 1.41  1999/09/11 04:43:17  steve
 *  Support ternary and <= operators in vvm.
 *
 * Revision 1.40  1999/09/08 02:24:39  steve
 *  Empty conditionals (pmonta@imedia.com)
 *
 * Revision 1.39  1999/09/04 19:11:46  steve
 *  Add support for delayed non-blocking assignments.
 *
 * Revision 1.38  1999/09/04 01:57:15  steve
 *  Generate fake adder code in vvm.
 *
 * Revision 1.37  1999/09/01 20:46:19  steve
 *  Handle recursive functions and arbitrary function
 *  references to other functions, properly pass
 *  function parameters and save function results.
 *
 * Revision 1.36  1999/08/31 22:38:29  steve
 *  Elaborate and emit to vvm procedural functions.
 *
 * Revision 1.35  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 * Revision 1.34  1999/08/02 00:19:16  steve
 *  Get rid of excess set/init of NetESignal objects.
 *
 * Revision 1.33  1999/08/01 16:34:50  steve
 *  Parse and elaborate rise/fall/decay times
 *  for gates, and handle the rules for partial
 *  lists of times.
 *
 * Revision 1.32  1999/07/18 21:17:51  steve
 *  Add support for CE input to XNF DFF, and do
 *  complete cleanup of replaced design nodes.
 *
 * Revision 1.31  1999/07/17 03:39:11  steve
 *  simplified process scan for targets.
 *
 * Revision 1.30  1999/07/10 03:00:05  steve
 *  Proper initialization of registers.
 *
 * Revision 1.29  1999/07/10 01:02:08  steve
 *  Handle number constants as parameters.
 *
 * Revision 1.28  1999/07/07 04:20:57  steve
 *  Emit vvm for user defined tasks.
 *
 * Revision 1.27  1999/07/03 02:12:52  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.26  1999/06/24 04:21:45  steve
 *  Add the === and !== binary operators.
 *
 * Revision 1.25  1999/06/19 21:06:16  steve
 *  Elaborate and supprort to vvm the forever
 *  and repeat statements.
 *
 * Revision 1.24  1999/06/10 04:03:43  steve
 *  Do not bother trying to print lvalue name in comment.
 *
 * Revision 1.23  1999/06/09 03:00:06  steve
 *  Add support for procedural concatenation expression.
 *
 * Revision 1.22  1999/06/07 02:23:31  steve
 *  Support non-blocking assignment down to vvm.
 *
 * Revision 1.21  1999/05/12 04:03:19  steve
 *  emit NetAssignMem objects in vvm target.
 *
 * Revision 1.20  1999/05/03 01:51:29  steve
 *  Restore support for wait event control.
 *
 * Revision 1.19  1999/05/01 20:43:55  steve
 *  Handle wide events, such as @(a) where a has
 *  many bits in it.
 *
 *  Add to vvm the binary ^ and unary & operators.
 *
 *  Dump events a bit more completely.
 *
 * Revision 1.18  1999/05/01 02:57:53  steve
 *  Handle much more complex event expressions.
 *
 * Revision 1.17  1999/04/25 22:52:32  steve
 *  Generate SubSignal refrences in vvm.
 *
 * Revision 1.16  1999/04/22 04:56:58  steve
 *  Add to vvm proceedural memory references.
 *
 * Revision 1.15  1999/04/19 01:59:37  steve
 *  Add memories to the parse and elaboration phases.
 *
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

