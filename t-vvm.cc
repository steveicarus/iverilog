/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: t-vvm.cc,v 1.118 2000/03/18 23:22:37 steve Exp $"
#endif

# include  <iostream>
# include  <fstream>
# include  <strstream>
# include  <iomanip>
# include  <string>
# include  <typeinfo>
# include  <unistd.h>
# include  "netlist.h"
# include  "netmisc.h"
# include  "target.h"

  // Comparison for use in sorting algorithms.

struct less_verinum {
      bool operator() (const verinum&left, const verinum&right)
	    { return left.is_before(right); }
};

static string make_temp()
{
      static unsigned counter = 0;
      ostrstream str;
      str << "TMP" << counter << ends;
      counter += 1;
      return str.str();
}

class target_vvm : public target_t {

      friend class vvm_parm_rval;

    public:
      target_vvm();
      ~target_vvm();

      virtual void start_design(ostream&os, const Design*);
      virtual void scope(ostream&os, const NetScope*);
      virtual void signal(ostream&os, const NetNet*);
      virtual void memory(ostream&os, const NetMemory*);
      virtual void task_def(ostream&os, const NetTaskDef*);
      virtual void func_def(ostream&os, const NetFuncDef*);

      virtual void lpm_add_sub(ostream&os, const NetAddSub*);
      virtual void lpm_clshift(ostream&os, const NetCLShift*);
      virtual void lpm_compare(ostream&os, const NetCompare*);
      virtual void lpm_ff(ostream&os, const NetFF*);
      virtual void lpm_mult(ostream&os, const NetMult*);
      virtual void lpm_mux(ostream&os, const NetMux*);
      virtual void lpm_ram_dq(ostream&os, const NetRamDq*);

      virtual void logic(ostream&os, const NetLogic*);
      virtual void bufz(ostream&os, const NetBUFZ*);
      virtual void udp(ostream&os, const NetUDP*);
      virtual void net_assign_nb(ostream&os, const NetAssignNB*);
      virtual void net_case_cmp(ostream&os, const NetCaseCmp*);
      virtual void net_const(ostream&os, const NetConst*);
      virtual void net_event(ostream&os, const NetNEvent*);
      virtual bool process(ostream&os, const NetProcTop*);
      virtual void proc_assign(ostream&os, const NetAssign*);
      virtual void proc_assign_mem(ostream&os, const NetAssignMem*);
      virtual void proc_assign_nb(ostream&os, const NetAssignNB*);
      virtual void proc_assign_mem_nb(ostream&os, const NetAssignMemNB*);
      virtual bool proc_block(ostream&os, const NetBlock*);
      virtual void proc_case(ostream&os, const NetCase*net);
              void proc_case_fun(ostream&os, const NetCase*net);
      virtual void proc_condit(ostream&os, const NetCondit*);
              void proc_condit_fun(ostream&os, const NetCondit*);
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
      void emit_init_value_(const NetObj::Link&lnk, verinum::V val);
      void emit_gate_outputfun_(const NetNode*, unsigned);
      string defn_gate_outputfun_(ostream&os, const NetNode*, unsigned);

	// This is the name of the thread (process or task) that is
	// being generated.
      string thread_class_;

	// This flag is true if we are writing out a function
	// definition. Thread steps are not available within
	// functions, and certain constructs are handled
	// differently. A flag is enough because function definitions
	// cannot nest.
      bool function_def_flag_;

	// Method definitions go into this file.
      char*defn_name;
      ofstream defn;

      char*init_code_name;
      ofstream init_code;

      char *start_code_name;
      ofstream start_code;

      unsigned process_counter;
      unsigned thread_step_;

	// These methods are use to help prefent duplicate printouts
	// of things that may be scanned multiple times.
      map<string,bool>esignal_printed_flag;
      map<string,bool>pevent_printed_flag;

      map<string,unsigned>nexus_wire_map;
      unsigned nexus_wire_counter;

	// String constants that are made into vpiHandles have th
	// handle name mapped by this.
      map<string,unsigned>string_constants;
      unsigned string_counter;

      map<verinum,unsigned,less_verinum>number_constants;
      unsigned number_counter;
};


target_vvm::target_vvm()
: function_def_flag_(false), init_code_name(0)
{
}

target_vvm::~target_vvm()
{
      assert(function_def_flag_ == false);
}

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
      virtual void expr_memory(const NetEMemory*mem);
      virtual void expr_signal(const NetESignal*);
      virtual void expr_subsignal(const NetESubSignal*sig);
      virtual void expr_ternary(const NetETernary*);
      virtual void expr_unary(const NetEUnary*);
      virtual void expr_binary(const NetEBinary*);
      virtual void expr_ufunc(const NetEUFunc*);
};

/*
 * Handle the concatenation operator in a procedural r-value
 * expression. Evaluate the concatenation into a temporary variable
 * with the right width, and return the name of that temporary as the
 * symbol that the context can use.
 */
void vvm_proc_rval::expr_concat(const NetEConcat*expr)
{
      assert(expr->repeat() > 0);
      string tname = make_temp();
      os_ << setw(indent_) << "" << "vvm_bitset_t<" <<
	    expr->expr_width() << "> " << tname << ";" << endl;

      unsigned pos = 0;
      for (unsigned rep = 0 ;  rep < expr->repeat() ;  rep += 1)
	    for (unsigned idx = 0 ;  idx < expr->nparms() ;  idx += 1) {

		  NetExpr*pp = expr->parm(expr->nparms() - idx - 1);
		  pp->expr_scan(this);

		  for (unsigned bit = 0 ; bit < pp->expr_width() ; bit += 1) {
			os_ << setw(indent_) << "" << tname << "[" << pos <<
			      "] = " << result << "[" << bit << "];" <<
			      endl;
			pos+= 1;
		  }
		  assert(pos <= expr->expr_width());
	    }

	/* Check that the positions came out to the right number of
	   bits. */
      if (pos != expr->expr_width()) {
	    os_ << "#error \"" << expr->get_line() << ": vvm eror: "
		  "width is " << expr->expr_width() << ", but I count "
		<< pos << " bits.\"" << endl;
      }

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

void vvm_proc_rval::expr_memory(const NetEMemory*mem)
{
      const string mname = mangle(mem->name());
      assert(mem->index());
      mem->index()->expr_scan(this);
      result = mname + ".get_word(" + result + ".as_unsigned())";
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
      os_ << "        " << mangle(expr->name()) << "();" << endl;

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

      os_ << "      vvm_bitset_t<" << expr->expr_width() << "> "
	  << tname << " = ";
      switch (expr->op()) {
	  case '~':
	    os_ << "vvm_unop_not(" << result << ");" << endl;
	    break;
	  case '&':
	    os_ << "vvm_unop_and(" << result << ");" << endl;
	    break;
	  case '|':
	    os_ << "vvm_unop_or(" << result << ");" << endl;
	    break;
	  case '^':
	    os_ << "vvm_unop_xor(" << result << ");" << endl;
	    break;
	  case '!':
	    os_ << "vvm_unop_lnot(" << result << ");" << endl;
	    break;
	  case '-':
	    os_ << "vvm_unop_uminus(" << result << ");" << endl;
	    break;
	  case 'N':
	    os_ << "vvm_unop_nor(" << result << ");" << endl;
	    break;
	  case 'X':
	    os_ << "vvm_unop_xnor(" << result << ");" << endl;
	    break;
	  default:
	    cerr << "vvm error: Unhandled unary op `" << expr->op() << "'"
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
      os_ << setw(indent_) << "" << "// " << expr->get_line() <<
	    ": expression node." << endl;
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
	    break;
	  case 'r': // right shift(>>)
	    os_ << setw(indent_) << "" << result << " = vvm_binop_shiftr("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'X':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_xnor("
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
	  case '*':
	    os_ << setw(indent_) << "" << "vvm_binop_mult(" << result
		<< "," << lres << "," << rres << ");" << endl;
	    break;
	  case '/':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_idiv("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case '%':
	    os_ << setw(indent_) << "" << result << " = vvm_binop_imod("
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
      explicit vvm_parm_rval(ostream&o, target_vvm*t)
      : result(""), os_(o), tgt_(t) { }

      string result;

    private:
      virtual void expr_const(const NetEConst*);
      virtual void expr_ident(const NetEIdent*);
      virtual void expr_memory(const NetEMemory*);
      virtual void expr_scope(const NetEScope*);
      virtual void expr_signal(const NetESignal*);

    private:
      ostream&os_;
      target_vvm*tgt_;
};

void vvm_parm_rval::expr_const(const NetEConst*expr)
{
      if (expr->value().is_string()) {

	    unsigned& res = tgt_->string_constants[expr->value().as_string()];

	    if (res == 0) {
		  res = tgt_->string_counter ++;
		  tgt_->init_code << "      vpip_make_string_const("
			"&string_table[" << res << "], \"" <<
			expr->value().as_string() << "\");" << endl;
	    }

	    ostrstream tmp;
	    tmp << "&string_table[" << res << "].base" << ends;
	    result = tmp.str();
	    return;
      }

      unsigned&res = tgt_->number_constants[expr->value()];
      if (res == 0) {
	    res = tgt_->number_counter ++;
	    unsigned width = expr->expr_width();
	    tgt_->init_code << "      { vpip_bit_t*bits = new vpip_bit_t["
			    << width << "];" << endl;

	    for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
		  tgt_->init_code << "        bits[" << idx << "] = ";
		  switch(expr->value().get(idx)) {
		      case verinum::V0:
			tgt_->init_code << "V0;" << endl;
			break;
		      case verinum::V1:
			tgt_->init_code << "V1;" << endl;
			break;
		      case verinum::Vx:
			tgt_->init_code << "Vx;" << endl;
			break;
		      case verinum::Vz:
			tgt_->init_code << "Vz;" << endl;
			break;
		  }
	    }
	    tgt_->init_code << "        vpip_make_number_const("
		  "&number_table[" << res << "], bits, " << width <<
		  ");" << endl;
	    tgt_->init_code << "      }" << endl;
      }

      ostrstream tmp;
      tmp << "&number_table[" << res << "].base" << ends;
      result = tmp.str();
      return;
}

void vvm_parm_rval::expr_ident(const NetEIdent*expr)
{
      if (expr->name() == "$time") {
	    result = string("vpip_sim_time()");
      } else {
	    cerr << "Unhandled identifier: " << expr->name() << endl;
      }
}

void vvm_parm_rval::expr_memory(const NetEMemory*mem)
{
      if (mem->index() == 0) {
	      /* If the expression is a memory without an index, then
		 return the handle for the memory object. System tasks
		 can take such things as parameters. */
	    result = string("&") + mangle(mem->name()) + ".base";

      } else if (const NetEConst*idx = dynamic_cast<const NetEConst*>(mem->index())){

	      /* If the expression is a memory with a constant index,
		 then generate a call to vpi_handle_by_index() to get
		 the memory word handle. */
	    unsigned long val = idx->value().as_ulong();
	    ostrstream res;
	    res << "vpi_handle_by_index(&" << mangle(mem->name()) <<
		  ".base, " << val << ")" << ends;
	    result = res.str();

      } else {

	      /* Otherwise, evaluate the index at run time and use
		 that to select the memory word. */
	    string rval = emit_proc_rval(tgt_->defn, 6, mem->index());
	    result = "vpi_handle_by_index(&" + mangle(mem->name()) +
		  ".base, " + rval + ".as_unsigned())";
      }
}

void vvm_parm_rval::expr_scope(const NetEScope*escope)
{
      result = string("&") + mangle(escope->scope()->name()) + "_scope.base";
}

void vvm_parm_rval::expr_signal(const NetESignal*expr)
{
      string res = string("&") + mangle(expr->name()) + ".base";
      result = res;
}

static string emit_parm_rval(ostream&os, target_vvm*tgt, const NetExpr*expr)
{
      vvm_parm_rval scan (os, tgt);
      expr->expr_scan(&scan);
      return scan.result;
}

void target_vvm::start_design(ostream&os, const Design*mod)
{
      defn_name = tempnam(0, "ivldf");
      defn.open(defn_name, ios::in | ios::out | ios::trunc);

      init_code_name = tempnam(0, "ivlic");
      init_code.open(init_code_name, ios::in | ios::out | ios::trunc);

      start_code_name = tempnam(0, "ivlsc");
      start_code.open(start_code_name, ios::in | ios::out | ios::trunc);

      os << "# include \"vvm.h\"" << endl;
      os << "# include \"vvm_nexus.h\"" << endl;
      os << "# include \"vvm_gates.h\"" << endl;
      os << "# include \"vvm_signal.h\"" << endl;
      os << "# include \"vvm_func.h\"" << endl;
      os << "# include \"vvm_calltf.h\"" << endl;
      os << "# include \"vvm_thread.h\"" << endl;
      os << "# include \"vpi_user.h\"" << endl;
      os << "# include \"vpi_priv.h\"" << endl;

      process_counter = 0;
      string_counter = 1;
      number_counter = 1;
      nexus_wire_counter = 1;

      init_code << "static void design_init()" << endl;
      init_code << "{" << endl;
      init_code << "      vpip_init_simulation();"
		<< endl;
      start_code << "static void design_start()" << endl;
      start_code << "{" << endl;
}

void target_vvm::scope(ostream&os, const NetScope*scope)
{
      string hname = mangle(scope->name()) + "_scope";
      os << "// SCOPE: " << scope->name() << endl;
      os << "static struct __vpiScope " << hname << ";" << endl;

      string type_code;
      switch (scope->type()) {
	  case NetScope::MODULE:
	    type_code = "vpiModule";
	    break;
	  case NetScope::TASK:
	    type_code = "vpiTask";
	    break;
	  case NetScope::FUNC:
	    type_code = "vpiFunction";
	    break;
	  case NetScope::BEGIN_END:
	    type_code = "vpiNamedBegin";
	    break;
	  case NetScope::FORK_JOIN:
	    type_code = "vpiNamedFork";
	    break;
      }

      init_code << "      vpip_make_scope(&" << hname << ", " <<
	    type_code << ", \"" << scope->name() << "\");" << endl;
}

void target_vvm::end_design(ostream&os, const Design*mod)
{
      os << "static struct __vpiStringConst string_table[" <<
	    string_counter+1 << "];" << endl;
      os << "static struct __vpiNumberConst number_table[" <<
	    number_counter+1 << "];" << endl;
      os << "static vvm_nexus_wire nexus_wire_table[" <<
	    nexus_wire_counter << "];" << endl;
      defn.close();

      os << "// **** Definition code" << endl;
      { ifstream rdefn (defn_name);
        os << rdefn.rdbuf();
      }
      unlink(defn_name);
      free(defn_name);
      defn_name = 0;
      os << "// **** end definition code" << endl;


      os << "// **** init_code" << endl;
      init_code << "}" << endl;
      init_code.close();
      { ifstream rinit_code (init_code_name);
        os << rinit_code.rdbuf();
      }
      unlink(init_code_name);
      free(init_code_name);
      init_code_name = 0;
      os << "// **** end init_code" << endl;


      os << "// **** start_code" << endl;
      start_code << "}" << endl;
      start_code.close();
      { ifstream rstart_code (start_code_name);
        os << rstart_code.rdbuf();
      }
      unlink(start_code_name);
      free(start_code_name);
      start_code_name = 0;
      os << "// **** end start_code" << endl;


      os << "main()" << endl << "{" << endl;
      string vpi_module_path = mod->get_flag("VPI_MODULE_PATH");
      if (vpi_module_path.length() > 0)
	    os << "      vvm_set_module_path(\"" << vpi_module_path <<
		  "\");" << endl;

      string vpi_module_list = mod->get_flag("VPI_MODULE_LIST");
      while (vpi_module_list.length()) {
	    string name;
	    unsigned pos = vpi_module_list.find(',');
	    if (pos < vpi_module_list.length()) {
		  name = vpi_module_list.substr(0, pos);
		  vpi_module_list = vpi_module_list.substr(pos+1);
	    } else {
		  name = vpi_module_list;
		  vpi_module_list = "";
	    }
	    os << "      vvm_load_vpi_module(\"" << name << ".vpi\");" << endl;
      }
      os << "      design_init();" << endl;
      os << "      design_start();" << endl;

      for (unsigned idx = 0 ;  idx < process_counter ;  idx += 1)
	    os << "      thread" << (idx+1) << "_t thread_" <<
		  (idx+1) << ";" << endl;

      os << "      vpip_simulation_run();" << endl;
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
      string net_name = mangle(sig->name());

      for (unsigned idx = 0 ;  idx < sig->pin_count() ;  idx += 1) {
	    string nexus = nexus_from_link(&sig->pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];
	    if (ncode == 0) {
		  nexus_wire_map[nexus] = ncode = nexus_wire_counter;
		  nexus_wire_counter += 1;
	    }

	    init_code << "      nexus_wire_table[" << ncode <<
		  "].connect(&" << net_name << ", " << idx << ");" << endl;
      }

      os << "static vvm_bitset_t<" << sig->pin_count() << "> " <<
	    net_name<< "_bits; /* " << sig->name() <<
	    " */" << endl;
      os << "static vvm_signal_t " << net_name << "(" << net_name <<
	    "_bits.bits, " << sig->pin_count() << ");" << endl;

      init_code << "      vpip_make_reg(&" << net_name <<
	    ", \"" << sig->name() << "\");" << endl;

      if (const NetScope*scope = sig->scope()) {
	    string sname = mangle(scope->name()) + "_scope";
	    init_code << "      vpip_attach_to_scope(&" << sname
		      << ", &" << net_name << ".base);" << endl;
      }


	/* Scan the signals of the vector, passing the initial value
	   to the inputs of all the connected devices. */
      for (unsigned idx = 0 ;  idx < sig->pin_count() ;  idx += 1) {
	    if (sig->get_ival(idx) == verinum::Vz)
		  continue;

	    init_code << "      " << mangle(sig->name()) << ".init_P("
		      << idx << ", V" << sig->get_ival(idx) << ");"
		      << endl;

	      // Propogate the initial value to inputs throughout.
	    emit_init_value_(sig->pin(idx), sig->get_ival(idx));
      }
}

void target_vvm::memory(ostream&os, const NetMemory*mem)
{
      const string mname = mangle(mem->name());
      os << "static vvm_memory_t<" << mem->width() << ", " <<
	    mem->count() << "> " << mname << ";"
	    " /* " << mem->name() << " */" << endl;
      init_code << "      vpip_make_memory(&" << mname << ", \"" <<
	    mem->name() << "\", " << mem->width() << ", " <<
	    mem->count() << ");" << endl;
}

void target_vvm::task_def(ostream&os, const NetTaskDef*def)
{
      thread_step_ = 0;
      const string name = mangle(def->name());
      const string save_thread_class = thread_class_;
      thread_class_ = name;

      os << "class " << name << "  : public vvm_thread {" << endl;
      os << "    public:" << endl;
      os << "      " << name << "(vvm_thread*th)" << endl;
      os << "      : vvm_thread(), back_(th), step_(&" << name <<
	    "::step_0_), callee_(0)" << endl;
      os << "      { }" << endl;
      os << "      ~" << name << "() { }" << endl;
      os << "      bool go() { return (this->*step_)(); }" << endl;
      os << "    private:" << endl;
      os << "      vvm_thread*back_;" << endl;
      os << "      bool (" << name << "::*step_)();" << endl;
      os << "      vvm_thread*callee_;" << endl;
      os << "      bool step_0_();" << endl;


      defn << "bool " << thread_class_ << "::step_0_() {" << endl;
      def->proc()->emit_proc(os, this);
      defn << "      back_ -> thread_yield();" << endl;
      defn << "      return false;" << endl;
      defn << "}" << endl;

      os << "};" << endl;
      thread_class_ = save_thread_class;
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

	// Flag that we are now in a function definition. Note that
	// function definitions cannot nest.
      assert(! function_def_flag_);
      function_def_flag_ = true;

      os << "// Function " << def->name() << endl;
      os << "static void " << name << "();" << endl;

      defn << "// Function " << def->name() << endl;
      defn << "static void " << name << "()" << endl;
      defn << "{" << endl;
      def->proc()->emit_proc(os, this);
      defn << "}" << endl;

      assert(function_def_flag_);
      function_def_flag_ = false;
}

string target_vvm::defn_gate_outputfun_(ostream&os,
					const NetNode*gate,
					unsigned gpin)
{
      assert(0);
      return "";
}

void target_vvm::emit_init_value_(const NetObj::Link&lnk, verinum::V val)
{
      map<string,bool>written;

      for (const NetObj::Link*cur = lnk.next_link()
		 ; (*cur) != lnk ;  cur = cur->next_link()) {

	    if (cur->get_dir() == NetObj::Link::OUTPUT)
		  continue;

	    if (! dynamic_cast<const NetObj*>(cur->get_obj()))
		  continue;

	      // Build an init statement for the link, that writes the
	      // value.
	    ostrstream line;
	    line << "      " << mangle(cur->get_obj()->name()) <<
		  ".init_" << cur->get_name() << "(" <<
		  cur->get_inst() << ", V" << val << ");" << endl << ends;


	      // Check to see if the line has already been
	      // written to. This can happen if the object is a
	      // NetESignal, because there can be many of them
	      // with the same name.
	    if (written[line.str()])
		  continue;

	    written[line.str()] = true;

	    init_code << line.str();
      }
}

/*
 * This method handles writing output functions for gates that have a
 * single output (at pin 0). This writes the output_fun method into
 * the delayed stream to be emitted to the output file later.
 */
void target_vvm::emit_gate_outputfun_(const NetNode*gate, unsigned gpin)
{
      assert(0);
}

void target_vvm::lpm_add_sub(ostream&os, const NetAddSub*gate)
{
      os << "static vvm_add_sub " <<
	    mangle(gate->name()) << "(" << gate->width() << ");" << endl;

	/* Connect the DataA inputs. */

      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    string nexus = nexus_from_link(&gate->pin_DataA(idx));
	    unsigned ncode = nexus_wire_map[nexus];
	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
		  mangle(gate->name()) << ", " <<
		  mangle(gate->name()) << ".key_DataA(" << idx <<
		  "));" << endl;
      }

	/* Connect the DataB inputs. */

      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    string nexus = nexus_from_link(&gate->pin_DataB(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
		  mangle(gate->name()) << ", " <<
		  mangle(gate->name()) << ".key_DataB(" << idx <<
		  "));" << endl;
      }

	/* Connect the outputs of the adder. */

      for (unsigned idx = 0 ; idx < gate->width() ;  idx += 1) {
	    string nexus = nexus_from_link(&gate->pin_Result(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(" <<
		  mangle(gate->name()) << ".config_rout(" << idx <<
		  "));" << endl;
      }

	// Connect the carry output if necessary.
      if (gate->pin_Cout().is_linked()) {
	    string nexus = nexus_from_link(&gate->pin_Cout());
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(" <<
		  mangle(gate->name()) << ".config_cout());" << endl;
      }

      if (gate->attribute("LPM_Direction") == "ADD") {
	    init_code << "      " <<  mangle(gate->name()) <<
		  ".init_Add_Sub(0, V1);" << endl;

      } else if (gate->attribute("LPM_Direction") == "SUB") {
	    init_code << "      " <<  mangle(gate->name()) <<
		  ".init_Add_Sub(0, V0);" << endl;

      }

      start_code << "      " << mangle(gate->name()) << ".start();" << endl;
}

void target_vvm::lpm_clshift(ostream&os, const NetCLShift*gate)
{
      string mname = mangle(gate->name());

      os << "static vvm_clshift " << mname << "(" <<
	    gate->width() << "," << gate->width_dist() << ");" <<
	    endl;

	/* Connect the Data input pins... */
      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    string nexus = nexus_from_link(&gate->pin_Data(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Data(" << idx << "));" << endl;
      }

	/* Connect the Distance input pins... */
      for (unsigned idx = 0 ;  idx < gate->width_dist() ;  idx += 1) {
	    string nexus = nexus_from_link(&gate->pin_Distance(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Distance(" << idx << "));" << endl;
      }

	/* Connect the output drivers to the nexus nodes. */
      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    string nexus = nexus_from_link(&gate->pin_Result(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }
}

void target_vvm::lpm_compare(ostream&os, const NetCompare*gate)
{
      string mname = mangle(gate->name());

      os << "static vvm_compare " << mname << "(" << gate->width() <<
	    ");" << endl;

	/* Connect DataA inputs... */
      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    string nexus = nexus_from_link(&gate->pin_DataA(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataA(" << idx
		      << "));" << endl;
      }

	/* Connect DataB inputs... */
      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    string nexus = nexus_from_link(&gate->pin_DataB(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataB(" << idx
		      << "));" << endl;
      }

      if (gate->pin_ALB().is_linked()) {
	    string nexus = nexus_from_link(&gate->pin_ALB());
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_ALB_out());" << endl;
      }

      if (gate->pin_AGB().is_linked()) {
	    string nexus = nexus_from_link(&gate->pin_AGB());
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_AGB_out());" << endl;
      }


      if (gate->pin_ALEB().is_linked()) {
	    string nexus = nexus_from_link(&gate->pin_ALEB());
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_ALEB_out());" << endl;
      }

      if (gate->pin_AGEB().is_linked()) {
	    string nexus = nexus_from_link(&gate->pin_AGEB());
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_AGEB_out());" << endl;
      }
}

void target_vvm::lpm_ff(ostream&os, const NetFF*gate)
{
      string nexus;
      unsigned ncode;
      string mname = mangle(gate->name());

      os << "static vvm_ff " << mname << "(" << gate->width() << ");" << endl;

	/* Connect the clock input... */

      nexus = nexus_from_link(&gate->pin_Clock());
      ncode = nexus_wire_map[nexus];

      init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		<< mname << ", " << mname << ".key_Clock());" << endl;

	/* Connect the Q output pins... */

      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    nexus = nexus_from_link(&gate->pin_Q(idx));
	    ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }


	/* Connect the Data input pins... */

      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    nexus = nexus_from_link(&gate->pin_Data(idx));
	    ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_Data(" << idx
		      << "));" << endl;
      }
}

void target_vvm::lpm_mult(ostream&os, const NetMult*mul)
{
      string mname = mangle(mul->name());

      os << "static vvm_mult " << mname << "(" << mul->width_r() <<
	    "," << mul->width_a() << "," << mul->width_b() << "," <<
	    mul->width_s() << ");" << endl;


	/* Connect the DataA inputs... */
      for (unsigned idx = 0 ;  idx < mul->width_a() ;  idx += 1) {
	    string nexus = nexus_from_link(&mul->pin_DataA(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataA("
		      << idx << "));" << endl;
      }

	/* Connect the Datab inputs... */
      for (unsigned idx = 0 ;  idx < mul->width_b() ;  idx += 1) {
	    string nexus = nexus_from_link(&mul->pin_DataB(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataB("
		      << idx << "));" << endl;
      }

	/* Connect the output pins... */
      for (unsigned idx = 0 ;  idx < mul->width_r() ;  idx += 1) {
	    string nexus = nexus_from_link(&mul->pin_Result(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }
}

void target_vvm::lpm_mux(ostream&os, const NetMux*mux)
{
      string mname = mangle(mux->name());

      os << "static vvm_mux " << mname << "(" << mux->width() << ","
	 << mux->size() << "," << mux->sel_width() << ");" << endl;

	/* Connect the select inputs... */
      for (unsigned idx = 0 ;  idx < mux->sel_width() ;  idx += 1) {
	    string nexus = nexus_from_link(&mux->pin_Sel(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Sel(" << idx << "));" << endl;
      }

	/* Connect the data inputs... */
      for (unsigned idx = 0 ;  idx < mux->size() ;  idx += 1) {
	    for (unsigned wid = 0 ;  wid < mux->width() ;  wid += 1) {
		  string nexus = nexus_from_link(&mux->pin_Data(wid, idx));
		  unsigned ncode = nexus_wire_map[nexus];

		  init_code << "      nexus_wire_table["<<ncode<<"]"
			    << ".connect(&" << mname << ", "
			    << mname << ".key_Data("
			    << wid << "," << idx << "));" << endl;
	    }
      }

	/* Connect the outputs... */
      for (unsigned idx = 0 ;  idx < mux->width() ;  idx += 1) {
	    string nexus = nexus_from_link(&mux->pin_Result(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }
}

void target_vvm::lpm_ram_dq(ostream&os, const NetRamDq*ram)
{
      string mname = mangle(ram->name());
      os << "static vvm_ram_dq<" << ram->width() << "," <<
	    ram->awidth() << "," << ram->size() << "> " << mname <<
	    "(&" << mangle(ram->mem()->name()) << ");" << endl;

      for (unsigned idx = 0 ;  idx < ram->width() ;  idx += 1) {
	    unsigned pin = ram->pin_Q(idx).get_pin();
	    string outfun = defn_gate_outputfun_(os, ram, pin);
	    init_code << "      " << mangle(ram->name()) <<
		  ".config_rout(" << idx << ", &" << outfun << ");" << endl;
	    emit_gate_outputfun_(ram, pin);
      }
}

void target_vvm::logic(ostream&os, const NetLogic*gate)
{

	/* Draw the definition of the gate object. The exact type to
	   use depends on the gate type. Whatever the type, the basic
	   format is the same for all the boolean gates. */

      switch (gate->type()) {
	  case NetLogic::AND:
	    if ((gate->pin_count()-1) == 2)
		  os << "static vvm_and2 ";
	    else
		  os << "static vvm_and" << "<" << gate->pin_count()-1 << "> ";
	    break;
	  case NetLogic::BUF:
	    os << "static vvm_buf ";
	    break;
	  case NetLogic::BUFIF0:
	    os << "static vvm_bufif0 ";
	    break;
	  case NetLogic::BUFIF1:
	    os << "static vvm_bufif1 ";
	    break;
	  case NetLogic::NAND:
	    os << "static vvm_nand" << "<" << gate->pin_count()-1 << "> ";
	    break;
	  case NetLogic::NOR:
	    if ((gate->pin_count()-1) == 2)
		  os << "static vvm_nor2 ";
	    else
		  os << "static vvm_nor" << "<" << gate->pin_count()-1 << "> ";
	    break;
	  case NetLogic::NOT:
	    os << "static vvm_not ";
	    break;
	  case NetLogic::OR:
	    os << "static vvm_or" << "<" << gate->pin_count()-1 << "> ";
	    break;
	  case NetLogic::XNOR:
	    os << "static vvm_xnor" << "<" << gate->pin_count()-1 << "> ";
	    break;
	  case NetLogic::XOR:
	    os << "static vvm_xor" << "<" << gate->pin_count()-1 << "> ";
	    break;
	  default:
	    os << "#error \"internal ivl error:bad gate type for " <<
		  gate->name() << "\"" << endl;
      }

      os << mangle(gate->name()) << " (" << gate->rise_time() << ");" << endl;

	/* Write the code to invoke startup for this object. */

      start_code << "      " << mangle(gate->name()) << ".start();" << endl;


	/* Connect the output and all the inputs of the gate to the
	   nexus objects, one bit at a time. */

      init_code << "      //  Connect inputs to gate " << gate->name()
		<< "." << endl;

      { string nexus = nexus_from_link(&gate->pin(0));
        unsigned ncode = nexus_wire_map[nexus];
	init_code << "      nexus_wire_table[" << ncode <<
	      "].connect(&" << mangle(gate->name()) << ");" << endl;
      }

      for (unsigned idx = 1 ;  idx < gate->pin_count() ;  idx += 1) {
	    string nexus = nexus_from_link(&gate->pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];
	    init_code << "      nexus_wire_table[" << ncode
		      << "].connect(&" << mangle(gate->name()) << ", "
		      << (idx-1) << ");" << endl;
      }
}

void target_vvm::bufz(ostream&os, const NetBUFZ*gate)
{
      string mname = mangle(gate->name());
      string nexus;
      unsigned ncode;

      os << "static vvm_bufz " << mname << ";" << endl;

      nexus = nexus_from_link(&gate->pin(0));
      ncode = nexus_wire_map[nexus];

      init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
	    mname << ");" << endl;

      nexus = nexus_from_link(&gate->pin(1));
      ncode = nexus_wire_map[nexus];

      init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
	    mname << ",0);" << endl;

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

      string outfun = defn_gate_outputfun_(os, gate, 0);

      os << "static vvm_udp_ssequ<" << gate->pin_count()-1 << "> " <<
	    mangle(gate->name()) << "(&" << outfun << ", V" <<
	    gate->get_initial() << ", " << mangle(gate->name()) <<
	    "_table);" << endl;

	/* The UDP output function is much like other logic gates. Use
	   this general method to output the output function. */
      emit_gate_outputfun_(gate, 0);

}

void target_vvm::net_assign_nb(ostream&os, const NetAssignNB*net)
{
}

void target_vvm::net_case_cmp(ostream&os, const NetCaseCmp*gate)
{
      string mname = mangle(gate->name());
      string nexus;
      unsigned ncode;

      assert(gate->pin_count() == 3);
      os << "static vvm_eeq " << mname << "(" <<
	    gate->rise_time() << ");" << endl;

	/* Connect the output pin */
      nexus = nexus_from_link(&gate->pin(0));
      ncode = nexus_wire_map[nexus];
      init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		<< mname << ");" << endl;

	/* Connect the first input */
      nexus = nexus_from_link(&gate->pin(1));
      ncode = nexus_wire_map[nexus];
      init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		<< mname << ", 0);" << endl;

	/* Connect the second input */
      nexus = nexus_from_link(&gate->pin(2));
      ncode  = nexus_wire_map[nexus];
      init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		<< mname << ", 1);" << endl;


      start_code << "      " << mname << ".start();" << endl;
}

/*
 * The NetConst is a synthetic device created to represent constant
 * values. I represent them in the output as a vvm_bufz object that
 * has its input connected to nothing but is initialized to the
 * desired constant value.
 */
void target_vvm::net_const(ostream&os, const NetConst*gate)
{
      for (unsigned idx = 0 ;  idx < gate->pin_count() ;  idx += 1)
	    emit_init_value_(gate->pin(idx), gate->value(idx));
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


	/* Connect this device as a receiver to the nexus that is my
	   source. Write the connect calls into the init code. */

      for (unsigned idx = 0 ;  idx < gate->pin_count() ;  idx += 1) {
	    string nexus = nexus_from_link(&gate->pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
		  mangle(gate->name()) << ", " << idx << ");" << endl;
      }
}

void target_vvm::start_process(ostream&os, const NetProcTop*proc)
{
      process_counter += 1;
      { ostrstream ts;
        ts << "thread" << process_counter << "_t" << ends;
	thread_class_ = ts.str();
      }
      thread_step_ = 0;

      os << "class " << thread_class_ << " : public vvm_thread {" << endl;

      os << "    public:" << endl;
      os << "      " << thread_class_ << "()" << endl;
      os << "      : vvm_thread(), step_(&" << thread_class_ <<
	    "::step_0_), callee_(0)" << endl;
      os << "      { }" << endl;
      os << "      ~" << thread_class_ << "() { }" << endl;
      os << endl;
      os << "      bool go() { return (this->*step_)(); }" << endl;
      os << "    private:" << endl;
      os << "      bool (" << thread_class_ << "::*step_)();" << endl;
      os << "      vvm_thread*callee_;" << endl;
      os << "      bool step_0_();" << endl;

      defn << "bool " << thread_class_ << "::step_0_() {" << endl;
}

/*
 * This method generates code for a procedural assignment. The lval is
 * a signal, but the assignment should generate code to go to all the
 * connected devices/events.
 */
void target_vvm::proc_assign(ostream&os, const NetAssign*net)
{

	/* Detect the very special (and very common) case that the
	   rvalue is a constant in this assignment. I this case, there
	   is no reason to go scan the expression, and in the process
	   generate bunches of temporaries. */

      if (const NetEConst*rc = dynamic_cast<const NetEConst*>(net->rval())) {
	    assert(net->bmux() == 0);
	    const verinum value = rc->value();

	    for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
		  string nexus = nexus_from_link(&net->pin(idx));
		  unsigned ncode = nexus_wire_map[nexus];
		  defn << "      nexus_wire_table[" <<ncode<< "].reg_assign(";
		  switch (value.get(idx)) {
		      case verinum::V0:
			defn << "V0";
			break;
		      case verinum::V1:
			defn << "V1";
			break;
		      case verinum::Vx:
			defn << "Vx";
			break;
		      case verinum::Vz:
			defn << "Vz";
			break;
		  }
		  defn << ");" << endl;
	    }
	    return;
      }

      string rval = emit_proc_rval(defn, 8, net->rval());

      defn << "      // " << net->get_line() << ": " << endl;

      if (net->bmux()) {

	      // This is a bit select. Assign the low bit of the rval
	      // to the selected bit of the lval.
	    string bval = emit_proc_rval(defn, 8, net->bmux());

	    defn << "      switch (" << bval << ".as_unsigned()) {" << endl;

	    for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {

		  string nexus = nexus_from_link(&net->pin(idx));
		  unsigned ncode = nexus_wire_map[nexus];

		  defn << "      case " << idx << ":" << endl;

		  defn << "        nexus_wire_table["<<ncode<<"]"
		       << ".reg_assign(" << rval << "[0]);" << endl;
		  defn << "        break;" << endl;

	    }

	    defn << "      }" << endl;

      } else {
	    for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
		  string nexus = nexus_from_link(&net->pin(idx));
		  unsigned ncode = nexus_wire_map[nexus];
		  defn << "      nexus_wire_table["<<ncode<<"].reg_assign("
		       << rval << "[" << idx << "]);" << endl;
	    }
      }
}

/*
 * Generate an assignment to a memory location. This causes the index
 * expression to be evaluated (run-time) and the index used as the
 * location to store the value.
 */
void target_vvm::proc_assign_mem(ostream&os, const NetAssignMem*amem)
{
      string index = mangle(amem->index()->name()) + "_bits";
      string rval = emit_proc_rval(defn, 8, amem->rval());
      const NetMemory*mem = amem->memory();

      defn << "      /* " << amem->get_line() << " */" << endl;
      if (mem->width() == amem->rval()->expr_width()) {
	    defn << "      " << mangle(mem->name()) <<
		  ".set_word(" << index << ".as_unsigned(), " <<
		  rval << ");" << endl;

      } else {
	    assert(mem->width() <= amem->rval()->expr_width());
	    string tmp = make_temp();
	    defn << "      vvm_bitset_t<" << mem->width() << ">" <<
		  tmp << ";" << endl;
	    for (unsigned idx = 0 ;  idx < mem->width() ;  idx += 1)
		  defn << "      " << tmp << "[" << idx << "] = " <<
			rval << "[" << idx << "];" << endl;

	    defn << "      " << mangle(mem->name()) << ".set_word("
		 << index << ".as_unsigned(), " << tmp << ");" << endl;
      }
}

void target_vvm::proc_assign_nb(ostream&os, const NetAssignNB*net)
{
      string rval = emit_proc_rval(defn, 8, net->rval());
      const unsigned long delay = net->rise_time();

      if (net->bmux()) {
	      /* If the l-value has a bit select, set the output bit
		 to only the desired bit. Evaluate the index and use
		 that to drive a switch statement.

		 XXXX I'm not fully satisfied with this, I might like
		 better generating a demux device and doing the assign
		 to the device input. Food for thought. */

	    string bval = emit_proc_rval(defn, 8, net->bmux());
	    defn << "      switch (" << bval << ".as_unsigned()) {" << endl;

	    for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
		  string nexus = nexus_from_link(&net->pin(idx));
		  unsigned ncode = nexus_wire_map[nexus];

		  defn << "      case " << idx << ":" << endl;
		  defn << "        vvm_delayed_assign(nexus_wire_table["
		       << ncode << "], " << rval << ", " << delay << ");"
		       << endl;
		  defn << "        break;" << endl;
	    }

	    defn << "      }" << endl;

      } else {
	    for (unsigned idx = 0 ; idx < net->pin_count() ;  idx += 1) {
		  string nexus = nexus_from_link(&net->pin(idx));
		  unsigned ncode = nexus_wire_map[nexus];
		  defn << "      vvm_delayed_assign(nexus_wire_table["
		       << ncode << "], " << rval << "[" << idx << "], "
		       << delay << ");" << endl;
	    }
      }
}

void target_vvm::proc_assign_mem_nb(ostream&os, const NetAssignMemNB*amem)
{

      string index = mangle(amem->index()->name()) + "_bits";
      string rval = emit_proc_rval(defn, 8, amem->rval());
      const NetMemory*mem = amem->memory();

      defn << "      /* " << amem->get_line() << " */" << endl;
      if (mem->width() == amem->rval()->expr_width()) {
	    defn << "      (new vvm_memory_t<" << mem->width() << ","
		 << mem->count() << ">::assign_nb(" << mangle(mem->name())
		 << ", " << index << ".as_unsigned(), " << rval <<
		  ")) -> schedule();" << endl;

      } else {

	    assert(mem->width() <= amem->rval()->expr_width());
	    string tmp = make_temp();
	    defn << "      vvm_bitset_t<" << mem->width() << ">" <<
		  tmp << ";" << endl;
	    for (unsigned idx = 0 ;  idx < mem->width() ;  idx += 1)
		  defn << "      " << tmp << "[" << idx << "] = " <<
			rval << "[" << idx << "];" << endl;

	    defn << "      (new vvm_memory_t<" << mem->width() << ","
		 << mem->count() << ">::assign_nb(" << mangle(mem->name())
		 << ", " << index << ".as_unsigned(), " << tmp <<
		  ")) -> schedule();" << endl;
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
      if (function_def_flag_) {
	    proc_case_fun(os, net);
	    return;
      }

      string test_func = "";
      switch (net->type()) {
	  case NetCase::EQ:
	    test_func = "vvm_binop_eeq";
	    break;
	  case NetCase::EQX:
	    test_func = "vvm_binop_xeq";
	    break;
	  case NetCase::EQZ:
	    test_func = "vvm_binop_zeq";
	    break;
      }

      defn << "      /* case (" << *net->expr() << ") */" << endl;
      string expr = emit_proc_rval(defn, 8, net->expr());

      unsigned exit_step = thread_step_ + 1;
      thread_step_ += 1;

      unsigned default_idx = net->nitems();

	/* This iteration generates all the comparisons with the case
	   expression. If a comparison matches, the next step is set
	   and return true branches to that step. */
      for (unsigned idx = 0 ;  idx < net->nitems() ;  idx += 1) {
	    if (net->expr(idx) == 0) {
		  assert(default_idx == net->nitems());
		  default_idx = idx;
		  continue;
	    }
	    assert(net->expr(idx));

	    thread_step_ += 1;

	    defn << "      /* " << *net->expr(idx) << " */" << endl;
	    string guard = emit_proc_rval(defn, 8, net->expr(idx));

	    defn << "      if (V1 == " << test_func << "(" << guard << ","
	       << expr << ")[0]) {" << endl;
	    defn << "          step_ = &" << thread_class_ <<
		  "::step_" << thread_step_ << "_;" << endl;
	    defn << "          return true;" << endl;
	    defn << "      }" << endl;
      }

	/* If none of the above tests pass, then branch to the default
	   step (or the exit step if there is no default.) */
      if (default_idx < net->nitems()) {
	    thread_step_ += 1;

	    defn << "      /* default : */" << endl;
	    defn << "      step_ = &" << thread_class_ << "::step_" <<
		  thread_step_ << "_;" << endl;

      } else {
	    defn << "      /* no default ... fall out of case. */" << endl;
	    defn << "      step_ = &" << thread_class_ << "::step_" <<
		  exit_step << "_;" << endl;
      }
      defn << "      return true;" << endl;
      defn << "}" << endl;

	/* Run through the cases again, this time generating the case
	   steps. Note that I already know which item is the default,
	   so I just assert that this iteration agrees. */
      unsigned step_num = exit_step;
      for (unsigned idx = 0 ;  idx < net->nitems() ;  idx += 1) {
	    if (net->expr(idx) == 0) {
		  assert(default_idx == idx);
		  continue;
	    }
	    assert(net->expr(idx));

	    step_num += 1;

	    os << "      bool step_" << step_num << "_();" << endl;

	    defn << "bool " << thread_class_ << "::step_" << step_num
		 << "_() {" << endl;
	    if (net->stat(idx))
		  net->stat(idx)->emit_proc(os, this);
	    defn << "      step_ = &" << thread_class_ << "::step_" <<
		  exit_step << "_;" << endl;
	    defn << "      return true;" << endl;
	    defn << "}" << endl;
      }

	/* If there is a default case, generate the default step. */
      if (default_idx < net->nitems()) {
	    step_num += 1;

	    os << "      bool step_" << step_num << "_();" << endl;

	    defn << "bool " << thread_class_ << "::step_" << step_num
		 << "_() {" << endl;
	    if (net->stat(default_idx))
		  net->stat(default_idx)->emit_proc(os, this);
	    defn << "      step_ = &" << thread_class_ << "::step_" <<
		  exit_step << "_;" << endl;
	    defn << "      return true;" << endl;
	    defn << "}" << endl;
      }

	/* Finally, start the exit step. */
      os << "      bool step_" << exit_step << "_();" << endl;

      defn << "bool " << thread_class_ << "::step_" << exit_step <<
	    "_() {" << endl;
}

/*
 * Within a function definition, the case statement is implemented
 * differently. Since statements in this context cannot block, we can
 * use open coded if statements to do all the comparisons.
 */
void target_vvm::proc_case_fun(ostream&os, const NetCase*net)
{
      string test_func = "";
      switch (net->type()) {
	  case NetCase::EQ:
	    test_func = "vvm_binop_eeq";
	    break;
	  case NetCase::EQX:
	    test_func = "vvm_binop_xeq";
	    break;
	  case NetCase::EQZ:
	    test_func = "vvm_binop_zeq";
	    break;
      }

      defn << "      /* " << net->get_line() << ": case (" <<
	    *net->expr() << ") */" << endl;

      defn << "      do {" << endl;

      string expr = emit_proc_rval(defn, 6, net->expr());

      unsigned default_idx = net->nitems();
      for (unsigned idx = 0 ;  idx < net->nitems() ;  idx += 1) {

	      // don't emit the default case here. Save it for the
	      // last else clause.
	    if (net->expr(idx) == 0) {
		  default_idx = idx;
		  continue;
	    }

	    string guard = emit_proc_rval(defn, 6, net->expr(idx));

	    defn << "      if (V1 == " << test_func << "(" <<
		  guard << "," << expr << ")[0]) {" << endl;
	    if (net->stat(idx))
		  net->stat(idx)->emit_proc(os, this);
	    defn << "      break; }" << endl;
      }

      if ((default_idx < net->nitems()) && net->stat(default_idx)) {
	    net->stat(default_idx)->emit_proc(os, this);
      }

      defn << "      /* " << net->get_line() << ": end case (" <<
	    *net->expr() << ") */" << endl;
      defn << "      } while(0);" << endl;
}

void target_vvm::proc_condit(ostream&os, const NetCondit*net)
{
      if (function_def_flag_) {
	    proc_condit_fun(os, net);
	    return;
      }

      string expr = emit_proc_rval(defn, 8, net->expr());

      unsigned if_step   = ++thread_step_;
      unsigned else_step = ++thread_step_;
      unsigned out_step  = ++thread_step_;

      defn << "      if (" << expr << "[0] == V1)" << endl;
      defn << "        step_ = &" << thread_class_ << "::step_" <<
	    if_step << "_;" << endl;
      defn << "      else" << endl;
      defn << "        step_ = &" << thread_class_ << "::step_" <<
	    else_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      os << "      bool step_" << if_step << "_();" << endl;

      defn << "bool " << thread_class_ << "::step_" << if_step <<
	    "_() {" << endl;
      net->emit_recurse_if(os, this);
      defn << "      step_ = &" << thread_class_ << "::step_" <<
	    out_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      os << "      bool step_" << else_step << "_();" << endl;

      defn << "bool " << thread_class_ << "::step_" << else_step <<
	    "_() {" << endl;
      net->emit_recurse_else(os, this);
      defn << "      step_ = &" << thread_class_ << "::step_" <<
	    out_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      os << "      bool step_" << out_step << "_();" << endl;

      defn << "bool " << thread_class_ << "::step_" << out_step <<
	    "_() {" << endl;
}

void target_vvm::proc_condit_fun(ostream&os, const NetCondit*net)
{
      string expr = emit_proc_rval(defn, 8, net->expr());

      defn << "      // " << net->get_line() << ": conditional (if-else)"
	   << endl;
      defn << "      if (" << expr << "[0] == V1) {" << endl;
      net->emit_recurse_if(os, this);
      defn << "      } else {" << endl;
      net->emit_recurse_else(os, this);
      defn << "      }" << endl;
}

/*
 * The forever loop is implemented by starting a basic block, handing
 * the statement, and putting in a goto to the beginning of the block.
 */
void target_vvm::proc_forever(ostream&os, const NetForever*net)
{
      unsigned top_step = ++thread_step_;
      unsigned out_step = ++thread_step_;

      defn << "      step_ = &" << thread_class_ << "::step_" <<
	    top_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;
      os << "      bool step_" << top_step << "_();" << endl;
      defn << "bool " << thread_class_ << "::step_" << top_step <<
	    "_() {" << endl;
      net->emit_recurse(os, this);
      defn << "      step_ = &" << thread_class_ << "::step_" <<
	    top_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      os << "      bool step_" << out_step << "_();" << endl;
      defn << "bool " << thread_class_ << "::step_" << out_step <<
	    "_() {" << endl;
}

void target_vvm::proc_repeat(ostream&os, const NetRepeat*net)
{
      string expr = emit_proc_rval(defn, 8, net->expr());
      unsigned top_step = ++thread_step_;
      unsigned out_step = ++thread_step_;

      defn << "      step_" << top_step << "_idx_ = " << expr <<
	    ".as_unsigned();" << endl;
      defn << "      step_ = &" << thread_class_ << "::step_" <<
	    top_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      os << "      unsigned step_" << top_step << "_idx_;" << endl;
      os << "      bool step_" << top_step << "_();" << endl;

      defn << "bool " << thread_class_ << "::step_" << top_step <<
	    "_() {" << endl;
      defn << "      if (step_" << top_step << "_idx_ == 0) {" << endl;
      defn << "        step_ = &" << thread_class_ << "::step_" <<
	    out_step << "_;" << endl;
      defn << "        return true;" << endl;
      defn << "      }" << endl;
      defn << "      step_" << top_step << "_idx_ -= 1;" << endl;

      net->emit_recurse(os,this);

      defn << "      step_ = &" << thread_class_ << "::step_" <<
	    top_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      os << "      bool step_" << out_step << "_();" << endl;

      defn << "bool " << thread_class_ << "::step_" << out_step <<
	    "_() {" << endl;
}

/*
 * Calls to system tasks are done here. We know that this is a system
 * task and that I need to generate an external call. Calls to user
 * defined tasks are handled elsewhere.
 */
void target_vvm::proc_stask(ostream&os, const NetSTask*net)
{
      string ptmp = make_temp();

      defn << "      vpiHandle " << ptmp << "[" << net->nparms() <<
	    "];" << endl;
      for (unsigned idx = 0 ;  idx < net->nparms() ;  idx += 1) {
	    string val;
	    if (net->parm(idx)) {
		  val = emit_parm_rval(os, this, net->parm(idx));

	    } else {
		  val = string("&vpip_null.base");
	    }

	    defn << "      " << ptmp << "[" << idx << "] = " << val << ";"
	       << endl;
      }

      defn << "      vpip_calltask(\"" << net->name() << "\", " <<
	    net->nparms() << ", " << ptmp << ");" << endl;
      defn << "      if (vpip_finished()) return false;" << endl;
}

void target_vvm::proc_utask(ostream&os, const NetUTask*net)
{
      unsigned out_step = ++thread_step_;
      const string name = mangle(net->name());
      defn << "      assert(callee_ == 0);" << endl;
      defn << "      callee_ = new " << name << "(this);" << endl;
      defn << "      step_ = &" << thread_class_ << "::step_" <<
	    out_step << "_;" << endl;
      defn << "      return false;" << endl;
      defn << "}" << endl;

      os << "      bool step_" << out_step << "_();" << endl;
      defn << "bool " << thread_class_ << "::step_" << out_step <<
	    "_() {" << endl;
      defn << "      delete callee_;" << endl;
      defn << "      callee_ = 0;" << endl;
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

      defn << "      step_ = &" << thread_class_ << "::step_" <<
	    head_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      os << "      bool step_" << head_step << "_();" << endl;

      defn << "// " << net->expr()->get_line() <<
	    ": top of while condition." << endl;
      defn << "bool " << thread_class_ << "::step_" << head_step <<
	    "_() {" << endl;

      string expr = emit_proc_rval(defn, 8, net->expr());

      defn << "// " << net->expr()->get_line() <<
	    ": test while condition." << endl;
      defn << "      if (" << expr << "[0] != V1) {" << endl;
      defn << "          step_ = &" << thread_class_ << "::step_" <<
	    out_step << "_;" << endl;
      defn << "          return true;" << endl;
      defn << "      }" << endl;

      net->emit_proc_recurse(os, this);

      defn << "// " << net->expr()->get_line() <<
	    ": end of while loop." << endl;
      defn << "      step_ = &" << thread_class_ << "::step_" <<
	    head_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      os << "      bool step_" << out_step << "_();" << endl;

      defn << "bool " << thread_class_ << "::step_" << out_step <<
	    "_() {" << endl;
}

/*
 * Within a process, the proc_event is a statement that is blocked
 * until the event is signalled.
 */
void target_vvm::proc_event(ostream&os, const NetPEvent*proc)
{
      thread_step_ += 1;
      defn << "      step_ = &" << thread_class_ << "::step_" <<
	    thread_step_ << "_;" << endl;

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
	    defn << "      if (" << mangle((*list)[0]->name()) <<
		  ".get()[0]==V1) {" << endl;
	    defn << "         return true;" << endl;
	    defn << "      } else {" << endl;
	    defn << "         " << mangle(proc->name()) <<
		  ".wait(this);" << endl;
	    defn << "         return false;" << endl;
	    defn << "      }" << endl;
      } else {
	      /* The canonical wait for an edge puts the thread into
		 the correct wait object, then returns false from the
		 thread to suspend execution. When things are ready to
		 proceed, the correct vvm_pevent will send a wakeup to
		 start the next basic block. */
	    defn << "      " << mangle(proc->name()) << ".wait(this);" << endl;
	    defn << "      return false;" << endl;
      }

      defn << "}" << endl;

      os << "      bool step_" << thread_step_ << "_();" << endl;

      defn << "bool " << thread_class_ << "::step_" << thread_step_ <<
	    "_() {" << endl;

      proc->emit_proc_recurse(os, this);
      delete list;
}

/*
 * A delay suspends the thread for a period of time.
 */
void target_vvm::proc_delay(ostream&os, const NetPDelay*proc)
{
      thread_step_ += 1;
      defn << "      step_ = &" << thread_class_ << "::step_" <<
	    thread_step_ << "_;" << endl;
      defn << "      thread_yield(" << proc->delay() << ");" << endl;
      defn << "      return false;" << endl;
      defn << "}" << endl;

      os << "      bool step_" << thread_step_ << "_();" << endl;

      defn << "bool " << thread_class_ << "::step_" << thread_step_ <<
	    "_() {" << endl;

      proc->emit_proc_recurse(os, this);
}

void target_vvm::end_process(ostream&os, const NetProcTop*proc)
{
      if (proc->type() == NetProcTop::KALWAYS) {
	    defn << "      step_ = &" << thread_class_ << "::step_0_;"
	       << endl;
	    defn << "      return true;" << endl;
      } else {
	    defn << "      step_ = 0;" << endl;
	    defn << "      return false;" << endl;
      }

      defn << "}" << endl;
      os << "};" << endl;
}


static target_vvm target_vvm_obj;

extern const struct target tgt_vvm = {
      "vvm",
      &target_vvm_obj
};
/*
 * $Log: t-vvm.cc,v $
 * Revision 1.118  2000/03/18 23:22:37  steve
 *  Update the FF device to nexus style.
 *
 * Revision 1.117  2000/03/18 02:26:02  steve
 *  Update bufz to nexus style.
 *
 * Revision 1.116  2000/03/18 01:26:59  steve
 *  Generate references into a table of nexus objects instead of
 *  generating lots of isolated nexus objects. Easier on linkers
 *  and compilers,
 *
 *  Add missing nexus support for l-value bit selects,
 *
 *  Detemplatize the vvm_mux type.
 *
 *  Fix up the vvm_nexus destructor to disconnect from drivers.
 *
 * Revision 1.115  2000/03/17 20:21:14  steve
 *  Detemplatize the vvm_signal_t class.
 *
 * Revision 1.114  2000/03/17 19:23:59  steve
 *  nor2 and and2 optimized gates.
 *
 * Revision 1.113  2000/03/17 17:25:53  steve
 *  Adder and comparator in nexus style.
 *
 * Revision 1.112  2000/03/17 03:05:13  steve
 *  Update vvm_mult to nexus style.
 *
 * Revision 1.111  2000/03/17 02:22:03  steve
 *  vvm_clshift implementation without templates.
 *
 * Revision 1.110  2000/03/16 23:13:49  steve
 *  Update LPM_MUX to nexus style.
 *
 * Revision 1.109  2000/03/16 21:47:27  steve
 *  Update LMP_CLSHIFT to use nexus interface.
 *
 * Revision 1.108  2000/03/16 19:03:03  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 */

