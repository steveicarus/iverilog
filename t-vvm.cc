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
#ident "$Id: t-vvm.cc,v 1.157 2000/06/13 03:24:48 steve Exp $"
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

/*
 * The generated code puts constants in behavioral expressions into a
 * bit number table. Every place in the code where a vvm_bits_t is
 * created, it references an offset and size into this table. the
 * table is collected as numbers are encountered, overlapping values
 * as much as possible so that the number of output bits is reduced.
 */
class NumberTable {

    public:
      NumberTable();
      ~NumberTable();

      unsigned position(const verinum&val);
      unsigned count() const { return nbits_; }
      verinum::V bit(unsigned idx) const { return bits_[idx]; }

    private:
      verinum::V*bits_;
      unsigned nbits_;
};

NumberTable::NumberTable()
{
      bits_ = 0;
      nbits_ = 0;
}

NumberTable::~NumberTable()
{
      if (bits_) delete[]bits_;
}

unsigned NumberTable::position(const verinum&val)
{
      if (bits_ == 0) {
	    nbits_ = val.len();
	    bits_ = new verinum::V[val.len()];
	    for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
		  bits_[idx] = val.get(idx);

	    return 0;
      }

	/* Look for a complete match. If I find one, then return the
	   index of the start and all done. */
      for (unsigned idx = 0 ;  (idx+val.len()) < nbits_ ;  idx += 1) {

	    bool match_flag = true;
	    for (unsigned bit = 0 ;  bit < val.len() ;  bit += 1)
		  if (bits_[idx+bit] != val.get(bit)) {
			match_flag = false;
			break;
		  }

	    if (match_flag)
		  return idx;
      }

      unsigned tail_match = 0;
      for (unsigned idx = 1; (idx < val.len()) && (idx < nbits_) ; idx += 1) {

	    bool match_flag = true;
	    for (unsigned bit = 0 ;  bit < idx ;  bit += 1)
		  if (bits_[nbits_-idx+bit] != val.get(bit)) {
			match_flag = false;
			break;
		  }

	    if (match_flag)
		  tail_match = idx;
      }

      unsigned ntmp = nbits_+val.len()-tail_match;
      verinum::V*tmp = new verinum::V[ntmp];
      for (unsigned idx = 0 ;  idx < nbits_ ;  idx += 1)
	    tmp[idx] = bits_[idx];

      for (unsigned idx = nbits_ ;  idx < ntmp ;  idx += 1)
	    tmp[idx] = val.get(idx-nbits_+tail_match);

      unsigned rc = nbits_ - tail_match;

      delete[]bits_;
      bits_ = tmp;
      nbits_ = ntmp;

      return rc;
}


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
      virtual void event(ostream&os, const NetEvent*);
      virtual void signal(ostream&os, const NetNet*);
      virtual void memory(ostream&os, const NetMemory*);
      virtual void task_def(ostream&os, const NetTaskDef*);
      virtual void func_def(ostream&os, const NetFuncDef*);

      virtual void lpm_add_sub(ostream&os, const NetAddSub*);
      virtual void lpm_clshift(ostream&os, const NetCLShift*);
      virtual void lpm_compare(ostream&os, const NetCompare*);
      virtual void lpm_divide(ostream&os, const NetDivide*);
      virtual void lpm_ff(ostream&os, const NetFF*);
      virtual void lpm_mult(ostream&os, const NetMult*);
      virtual void lpm_mux(ostream&os, const NetMux*);
      virtual void lpm_ram_dq(ostream&os, const NetRamDq*);

      virtual void logic(ostream&os, const NetLogic*);
      virtual void bufz(ostream&os, const NetBUFZ*);
      virtual void udp(ostream&os, const NetUDP*);
      virtual void udp_comb(ostream&os, const NetUDP_COMB*);
              void udp_sequ_(ostream&os, const NetUDP*);
      virtual void net_assign_nb(ostream&os, const NetAssignNB*);
      virtual void net_case_cmp(ostream&os, const NetCaseCmp*);
      virtual bool net_cassign(ostream&os, const NetCAssign*);
      virtual void net_const(ostream&os, const NetConst*);
      virtual bool net_force(ostream&os, const NetForce*);
      virtual void net_probe(ostream&os, const NetEvProbe*);
      virtual bool process(ostream&os, const NetProcTop*);
      virtual void proc_assign(ostream&os, const NetAssign*);
      virtual void proc_assign_mem(ostream&os, const NetAssignMem*);
      virtual void proc_assign_nb(ostream&os, const NetAssignNB*);
      virtual void proc_assign_mem_nb(ostream&os, const NetAssignMemNB*);
      virtual bool proc_block(ostream&os, const NetBlock*);
      virtual void proc_case(ostream&os, const NetCase*net);
              void proc_case_fun(ostream&os, const NetCase*net);
      virtual bool proc_cassign(ostream&os, const NetCAssign*);
      virtual void proc_condit(ostream&os, const NetCondit*);
              void proc_condit_fun(ostream&os, const NetCondit*);
      virtual bool proc_deassign(ostream&os, const NetDeassign*);
      virtual bool proc_force(ostream&os, const NetForce*);
      virtual void proc_forever(ostream&os, const NetForever*);
      virtual bool proc_release(ostream&os, const NetRelease*);
      virtual void proc_repeat(ostream&os, const NetRepeat*);
      virtual void proc_stask(ostream&os, const NetSTask*);
      virtual bool proc_trigger(ostream&os, const NetEvTrig*);
      virtual void proc_utask(ostream&os, const NetUTask*);
      virtual bool proc_wait(ostream&os, const NetEvWait*);
      virtual void proc_while(ostream&os, const NetWhile*);
      virtual void proc_delay(ostream&os, const NetPDelay*);
      virtual void end_design(ostream&os, const Design*);

      void start_process(ostream&os, const NetProcTop*);
      void end_process(ostream&os, const NetProcTop*);


      NumberTable bits_table;

    private:
      void emit_init_value_(const Link&lnk, verinum::V val);
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

      unsigned signal_bit_counter;

      map<string,unsigned>nexus_wire_map;
      unsigned nexus_wire_counter;

	// String constants that are made into vpiHandles have th
	// handle name mapped by this.
      map<string,unsigned>string_constants;
      unsigned string_counter;

      map<verinum,unsigned,less_verinum>number_constants;
      unsigned number_counter;

      unsigned selector_counter;
};

static const char*vvm_val_name(verinum::V val,
			       Link::strength_t drv0,
			       Link::strength_t drv1)
{
      switch (val) {
	  case verinum::V0:
	    switch (drv0) {
		case Link::HIGHZ:
		  return "HiZ";
		case Link::WEAK:
		  return "We0";
		case Link::PULL:
		  return "Pu0";
		case Link::STRONG:
		  return "St0";
		case Link::SUPPLY:
		  return "Su0";
	    }
	    break;

	  case verinum::V1:
	    switch (drv1) {
		case Link::HIGHZ:
		  return "HiZ";
		case Link::WEAK:
		  return "We1";
		case Link::PULL:
		  return "Pu1";
		case Link::STRONG:
		  return "St1";
		case Link::SUPPLY:
		  return "Su1";
	    }
	    break;

	  case verinum::Vx:
	    return "StX";

	  case verinum::Vz:
	    return "HiZ";
      }

	/* This should not happen! */
      return "?strength?";
}


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
 * specified. The result is a vvm_bitset_t or equivilent that can be
 * used for reading values out.
 */
class vvm_proc_rval  : public expr_scan_t {

    public:
      explicit vvm_proc_rval(ostream&os, target_vvm*t)
      : result(""), os_(os), tgt_(t) { }

      string result;

    private:
      ostream&os_;
      target_vvm*tgt_;

    private:
      virtual void expr_const(const NetEConst*);
      virtual void expr_concat(const NetEConcat*);
      virtual void expr_ident(const NetEIdent*);
      virtual void expr_memory(const NetEMemory*mem);
      virtual void expr_sfunc(const NetESFunc*);
      virtual void expr_signal(const NetESignal*);
      virtual void expr_subsignal(const NetESubSignal*sig);
      virtual void expr_ternary(const NetETernary*);
      virtual void expr_unary(const NetEUnary*);
      virtual void expr_binary(const NetEBinary*);
      virtual void expr_ufunc(const NetEUFunc*);
};

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
      virtual void expr_memory(const NetEMemory*);
      virtual void expr_scope(const NetEScope*);
      virtual void expr_sfunc(const NetESFunc*);
      virtual void expr_signal(const NetESignal*);

    private:
      ostream&os_;
      target_vvm*tgt_;
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

      os_ << "      vpip_bit_t " << tname << "_bits["
	  << expr->expr_width() << "];" << endl;
      os_ << "      vvm_bitset_t " << tname << "(" << tname << "_bits, "
	  << expr->expr_width() << ");" << endl;

      unsigned pos = 0;
      for (unsigned rep = 0 ;  rep < expr->repeat() ;  rep += 1)
	    for (unsigned idx = 0 ;  idx < expr->nparms() ;  idx += 1) {

		  NetExpr*pp = expr->parm(expr->nparms() - idx - 1);
		  pp->expr_scan(this);

		  for (unsigned bit = 0 ; bit < pp->expr_width() ; bit += 1) {
			os_ << "      " << tname << "[" << pos <<
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

      unsigned number_off = tgt_->bits_table.position(expr->value());
      os_ << "      vvm_bitset_t " << tname << "(bits_table+"
	  << number_off << ", " << expr->expr_width() << ");" << endl;

      result = tname;
}

void vvm_proc_rval::expr_ident(const NetEIdent*expr)
{
      result = mangle(expr->name());
}

/*
 * a bitset rval that is a memory reference.
 */
void vvm_proc_rval::expr_memory(const NetEMemory*mem)
{
	/* Make a temporary to hold the word from the memory. */
      const string tname = make_temp();
      os_ << "      vpip_bit_t " << tname << "_bits["
	  << mem->expr_width() << "];" << endl;
      os_ << "      vvm_bitset_t " << tname << "(" << tname << "_bits, "
	  << mem->expr_width() << ");" << endl;

      const string mname = mangle(mem->name());

	/* Evaluate the memory index */
      assert(mem->index());
      mem->index()->expr_scan(this);


	/* Write code to use the calculated index to get the word from
	   the memory into the temporary we created earlier. */

      os_ << "      " << mname << ".get_word(" <<
	    result << ".as_unsigned(), " << tname << ");" << endl;

      result = tname;
}

void vvm_proc_rval::expr_sfunc(const NetESFunc*fun)
{
      os_ << "      // " << fun->get_line() << endl;

      const string retval = make_temp();
      const unsigned retwid = fun->expr_width();

	/* Make any parameters that might be needed to be passed to
	   the function. */

      const string parmtab = make_temp();
      if (fun->nparms() > 0) {
	    os_ << "      vpiHandle " << parmtab
		<< "["<<fun->nparms()<<"];" << endl;

	    for (unsigned idx = 0 ;  idx < fun->nparms() ;  idx += 1) {
		  vvm_parm_rval scan(os_, tgt_);
		  fun->parm(idx)->expr_scan(&scan);

		  os_ << "      " << parmtab <<"["<<idx<<"] = "
		      << scan.result << ";" << endl;
	    }
      }

	/* Draw the call to the function. Create a vpip_bit_t array to
	   receive the return value, and make it into a vvm_bitset_t
	   when the call returns. */

      os_ << "      vpip_bit_t " << retval << "_bits["<<retwid<<"];" << endl;

      os_ << "      vpip_callfunc(\"" << fun->name() << "\", "
	  << retwid << ", " << retval<<"_bits";

      if (fun->nparms() == 0)
	    os_ << ", 0, 0";
      else
	    os_ << ", " << fun->nparms() << ", " << parmtab;

      os_ << ");" << endl;

      os_ << "      vvm_bitset_t " << retval << "(" << retval<<"_bits, "
	  << retwid << ");" << endl;

      result = retval;
}

/*
 * A bitset reference to a signal can be done simply by referring to
 * the same bits as the signal. We onlt need to copy the bits pointer
 * from the vvm_signal_t object to get our reference.
 */
void vvm_proc_rval::expr_signal(const NetESignal*expr)
{
      const string tname = make_temp();

      os_ << "      vvm_bitset_t " << tname << "("
	  << mangle(expr->name()) << ".bits, "
	  << expr->expr_width() << ");" << endl;

      result = tname;
}

void vvm_proc_rval::expr_subsignal(const NetESubSignal*sig)
{
      string idx = make_temp();
      string val = make_temp();
      if (const NetEConst*cp = dynamic_cast<const NetEConst*>(sig->index())) {
	    os_ << "      const unsigned " << idx <<
		  " = " << cp->value().as_ulong() << ";" << endl;

      } else {
	    sig->index()->expr_scan(this);
	    os_ << "      const unsigned " <<
		  idx << " = " << result << ".as_unsigned();" <<
		  endl;
      }

	/* Get the bit select of a signal by making a vvm_bitset_t
	   object that refers to the single bit within the signal that
	   is of interest. */

      os_ << "      vvm_bitset_t " << val << "("
	  << mangle(sig->name()) << ".bits+" << idx << ", 1);" << endl;

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

      os_ << "      vpip_bit_t " << result << "_bits["
	  << expr->expr_width() << "];" << endl;
      os_ << "      vvm_bitset_t " << result << "(" << result<<"_bits, "
	  << expr->expr_width() << ");" << endl;

      os_ << "      vvm_ternary(" << result << ", " << cond_val<<"[0], "
	  << true_val << ", " << false_val << ");" << endl;
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
	    assert(expr->parm(idx));
	    assert(def->port(idx+1));

	    expr->parm(idx)->expr_scan(this);
	    string bname = mangle(def->port(idx+1)->name());
	    for (unsigned bit = 0 ; 
		 bit < expr->parm(idx)->expr_width() ;  bit += 1) {

		  os_ << "      " << bname << ".bits["<<bit<<"] = " <<
			result << "["<<bit<<"];" << endl;
	    }
      }

	/* Make the function call. */
      os_ << "        " << mangle(expr->name()) << "();" << endl;

	/* rbits is the bits of the signal that hold the result. */
      string rbits = mangle(expr->result()->name()) + ".bits";

	/* Make a temporary to hold the result... */
      result = make_temp();
      os_ << "      vpip_bit_t " << result << "_bits["
	  << expr->expr_width() << "];" << endl;
      os_ << "      vvm_bitset_t " << result << "("
	  << result<<"_bits, " << expr->expr_width() << ");" << endl;

	/* Copy the result into the new temporary. */
      for (unsigned idx = 0 ;  idx < expr->expr_width() ;  idx += 1)
	    os_ << "      " << result << "_bits[" << idx << "] = "
		<< rbits << "[" << idx << "];" << endl;

}

void vvm_proc_rval::expr_unary(const NetEUnary*expr)
{
      expr->expr()->expr_scan(this);
      string tname = make_temp();

      os_ << "      vpip_bit_t " << tname << "_bits["
	  << expr->expr_width() << "];" << endl;
      os_ << "      vvm_bitset_t " << tname << "(" << tname<<"_bits, "
	  << expr->expr_width() << ");" << endl;

      switch (expr->op()) {
	  case '~':
	    os_ << "      vvm_unop_not(" << tname << "," << result <<
		  ");" << endl;
	    break;
	  case '&':
	    os_ << "      " << tname << "[0] "
		  "= vvm_unop_and("<<result<<");" << endl;
	    break;
	  case '|':
	    os_ << "      " << tname << "[0] "
		  "= vvm_unop_or("<<result<<");" << endl;
	    break;
	  case '^':
	    os_ << "      " << tname << "[0] "
		  "= vvm_unop_xor("<<result<<");" << endl;
	    break;
	  case '!':
	    os_ << "      " << tname << "[0] "
		  "= vvm_unop_lnot("<<result<<");" << endl;
	    break;
	  case '-':
	    os_ << "vvm_unop_uminus(" <<tname<< "," << result << ");" << endl;
	    break;
	  case 'N':
	    os_ << "      " << tname << "[0] "
		  "= vvm_unop_nor("<<result<<");" << endl;
	    break;
	  case 'X':
	    os_ << "      " << tname << "[0] "
		  "= vvm_unop_xnor("<<result<<");" << endl;
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

      assert(expr->expr_width() != 0);

      result = make_temp();
      os_ << "      // " << expr->get_line() << ": expression node." << endl;
      os_ << "      vpip_bit_t " << result<<"_bits[" << expr->expr_width()
	  << "];" << endl;
      os_ << "      vvm_bitset_t " << result << "(" << result << "_bits, "
	  << expr->expr_width() << ");" << endl;

      switch (expr->op()) {
	  case 'a': // logical and (&&)
	    os_ << "      " << result << "[0] = vvm_binop_land("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'E': // ===
	    os_ << "      " << result << "[0] = vvm_binop_eeq("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'e': // ==
	    os_ << "      " << result << "[0] = vvm_binop_eq("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'G': // >=
	    os_ << "      " << result << "[0] = vvm_binop_ge("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'l': // left shift(<<)
	    os_ << "      " << "vvm_binop_shiftl(" << result
		<< ", " << lres << "," << rres << ");" << endl;
	    break;
	  case 'L': // <=
	    os_ << "      " << result << "[0] = vvm_binop_le("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'N': // !==
	    os_ << "      " << result << "[0] = vvm_binop_nee("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'n':
	    os_ << "      " << result << "[0] = vvm_binop_ne("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case '<':
	    os_ << "      " << result << "[0] = vvm_binop_lt("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case '>':
	    os_ << "      " << result << "[0] = vvm_binop_gt("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'o': // logical or (||)
	    os_ << "      " << result << "[0] = vvm_binop_lor("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'r': // right shift(>>)
	    os_ << "      " << "vvm_binop_shiftr(" << result
		<< ", " << lres << "," << rres << ");" << endl;
	    break;
	  case 'X':
	    os_ << "      " << "vvm_binop_xnor(" << result
		<< ", " << lres << "," << rres << ");" << endl;
	    break;
	  case '+':
	    os_ << "      " << "vvm_binop_plus(" << result
		<< ", " << lres << "," << rres << ");" << endl;
	    break;
	  case '-':
	    os_ << "      " << "vvm_binop_minus(" << result
		<< ", " << lres << "," << rres << ");" << endl;
	    break;
	  case '&':
	    os_ << "      " << "vvm_binop_and(" << result
		<< ", " << lres << ", " << rres << ");" << endl;
	    break;
	  case '|':
	    os_ << "      " << "vvm_binop_or(" << result
		<< ", " << lres << ", " << rres << ");" << endl;
	    break;
	  case '^':
	    os_ << "      " << "vvm_binop_xor(" << result
		<< ", " << lres << ", " << rres << ");" << endl;
	    break;
	  case '*':
	    os_ << "      " << "vvm_binop_mult(" << result
		<< "," << lres << "," << rres << ");" << endl;
	    break;
	  case '/':
	    os_ << "      " << "vvm_binop_idiv(" << result
		<< "," << lres << "," << rres << ");" << endl;
	    break;
	  case '%':
	    os_ << "      " << "vvm_binop_imod(" << result
		<< "," << lres << "," << rres << ");" << endl;
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

static string emit_proc_rval(ostream&os, target_vvm*tgt, const NetExpr*expr)
{
      vvm_proc_rval scan (os, tgt);
      expr->expr_scan(&scan);
      return scan.result;
}

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
	    unsigned bit_idx = tgt_->bits_table.position(expr->value());
	    tgt_->init_code << "      vpip_make_number_const("
			    << "&number_table[" << res << "], "
			    << "bits_table+" << bit_idx << ", "
			    << width << ");" << endl;
      }

      ostrstream tmp;
      tmp << "&number_table[" << res << "].base" << ends;
      result = tmp.str();
      return;
}

void vvm_parm_rval::expr_sfunc(const NetESFunc*expr)
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
	    string rval = emit_proc_rval(tgt_->defn, tgt_, mem->index());
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

      signal_bit_counter = 0;
      process_counter = 0;
      string_counter = 1;
      number_counter = 1;
      nexus_wire_counter = 1;
      selector_counter = 0;

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

      if (const NetScope*par = scope->parent()) {
	    string pname = mangle(par->name()) + "_scope";
	    init_code << "      vpip_attach_to_scope(&" << pname << ", "
		      << "&" << hname << ".base);" << endl;
      }
}

void target_vvm::event(ostream&os, const NetEvent*event)
{
      string mname = mangle(event->full_name());
      os << "static vvm_sync " << mname << "; // "
	 << event->get_line() << ": event " << event->full_name() << endl;
}

void target_vvm::end_design(ostream&os, const Design*mod)
{
      if (string_counter > 0)
	    os << "static struct __vpiStringConst string_table[" <<
		  string_counter+1 << "];" << endl;
      if (number_counter > 0)
	    os << "static struct __vpiNumberConst number_table[" <<
		  number_counter+1 << "];" << endl;
      if (nexus_wire_counter > 0)
	    os << "static vvm_nexus nexus_wire_table[" <<
		  nexus_wire_counter << "];" << endl;
      if (signal_bit_counter > 0)
	    os << "static vpip_bit_t signal_bit_table[" <<
		  signal_bit_counter << "];" << endl;

      if (bits_table.count() > 0) {
	    os << "static vpip_bit_t bits_table[" << bits_table.count()
	       << "] = {";

	    for (unsigned idx = 0 ;  idx < bits_table.count() ;  idx += 1) {
		  if (idx%16 == 0) os << endl;
		  switch (bits_table.bit(idx)) {
		      case verinum::V0:
			os << " St0,";
			break;
		      case verinum::V1:
			os << " St1,";
			break;
		      case verinum::Vx:
			os << " StX,";
			break;
		      case verinum::Vz:
			os << " HiZ,";
			break;
		      default:
			os << "    ,";
			break;
		  }
	    }

	    os << endl << "};" << endl;
      }

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
	    bool new_nexus_flag = false;
	    string nexus = nexus_from_link(&sig->pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];
	    if (ncode == 0) {
		  nexus_wire_map[nexus] = (ncode = nexus_wire_counter);
		  nexus_wire_counter += 1;
		  new_nexus_flag = true;
	    }

	    init_code << "      nexus_wire_table[" << ncode <<
		  "].connect(&" << net_name << ", " << idx << ");" << endl;

	      // Propogate the initial value to inputs throughout.
	    if (new_nexus_flag)
		  emit_init_value_(sig->pin(idx), sig->get_ival(idx));
      }

      os << "static vvm_signal_t " << net_name << ";" << endl;

      init_code << "      vpip_make_reg(&" << net_name
		<< ", \"" << sig->name() << "\", signal_bit_table+"
		<< signal_bit_counter << ", " << sig->pin_count()
		<< ");" << endl;


      signal_bit_counter += sig->pin_count();

      if (! sig->local_flag()) {
	    const NetScope*scope = sig->scope();
	    assert(scope);
	    string sname = mangle(scope->name()) + "_scope";
	    init_code << "      vpip_attach_to_scope(&" << sname
		      << ", &" << net_name << ".base);" << endl;
      }


	/* Scan the signals of the vector, passing the initial value
	   to the inputs of all the connected devices. */
      for (unsigned idx = 0 ;  idx < sig->pin_count() ;  idx += 1) {

	    init_code << "      " << mangle(sig->name()) << ".init_P("
		      << idx << ", ";
	    switch (sig->get_ival(idx)) {
		case verinum::V0:
		  init_code << "St0";
		  break;
		case verinum::V1:
		  init_code << "St1";
		  break;
		case verinum::Vx:
		  init_code << "StX";
		  break;
		case verinum::Vz:
		  init_code << "HiZ";
		  break;
	    }
	    init_code << ");" << endl;
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

      os << "static bool " << name << "_step_0_(vvm_thread*thr);" << endl;

      defn << "static bool " << name << "_step_0_(vvm_thread*thr) {" << endl;

      def->proc()->emit_proc(os, this);

      defn << "      thr->back_ -> thread_yield();" << endl;
      defn << "      return false;" << endl;
      defn << "}" << endl;

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
      cerr << "internal error: outputfun_ called for gate " <<
	    gate->name() << " (" << typeid(*gate).name() << ")" <<
	    endl;
      assert(0);
      return "";
}

void target_vvm::emit_init_value_(const Link&lnk, verinum::V val)
{
      map<string,bool>written;
      const char*val_name = vvm_val_name(val, lnk.drive0(), lnk.drive1());

      for (const Link*cur = lnk.next_link()
		 ; (*cur) != lnk ;  cur = cur->next_link()) {

	    if (cur->get_dir() == Link::OUTPUT)
		  continue;

	    if (! dynamic_cast<const NetObj*>(cur->get_obj()))
		  continue;

	      /* If the caller lnk is a signal, then we are declaring
		 signals so we skip signal initializations as they
		 take care of themselves. */

	    if (dynamic_cast<const NetNet*>(cur->get_obj())
		&& dynamic_cast<const NetNet*>(lnk.get_obj()))
		  continue;

	      // Build an init statement for the link, that writes the
	      // value.
	    ostrstream line;
	    line << "      " << mangle(cur->get_obj()->name())
		 << ".init_" << cur->get_name() << "(" << cur->get_inst()
		 << ", " << val_name << ");" << endl << ends;


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
	    if (! gate->pin_DataA(idx).is_linked())
		  continue;

	    string nexus = nexus_from_link(&gate->pin_DataA(idx));
	    unsigned ncode = nexus_wire_map[nexus];
	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
		  mangle(gate->name()) << ", " <<
		  mangle(gate->name()) << ".key_DataA(" << idx <<
		  "));" << endl;
      }

	/* Connect the DataB inputs. */

      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    if (! gate->pin_DataB(idx).is_linked())
		  continue;

	    string nexus = nexus_from_link(&gate->pin_DataB(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
		  mangle(gate->name()) << ", " <<
		  mangle(gate->name()) << ".key_DataB(" << idx <<
		  "));" << endl;
      }

	/* Connect the outputs of the adder. */

      for (unsigned idx = 0 ; idx < gate->width() ;  idx += 1) {
	    if (! gate->pin_Result(idx).is_linked())
		  continue;

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
		  ".init_Add_Sub(0, St1);" << endl;

      } else if (gate->attribute("LPM_Direction") == "SUB") {
	    init_code << "      " <<  mangle(gate->name()) <<
		  ".init_Add_Sub(0, St0);" << endl;

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

	/* Connect the Direction pin... */
      if (gate->pin_Direction().is_linked()) {
	    string nexus = nexus_from_link(&gate->pin_Direction());
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Direction(0));" << endl;
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

void target_vvm::lpm_divide(ostream&os, const NetDivide*mul)
{
      string mname = mangle(mul->name());

      os << "static vvm_idiv " << mname << "(" << mul->width_r() <<
	    "," << mul->width_a() << "," << mul->width_b() << ");" << endl;


	/* Connect the DataA inputs... */
      for (unsigned idx = 0 ;  idx < mul->width_a() ;  idx += 1) {
	    string nexus = nexus_from_link(&mul->pin_DataA(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataA("
		      << idx << "));" << endl;
      }

	/* Connect the DataB inputs... */
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

      if (ram->pin_WE().is_linked()) {
	    string nexus = nexus_from_link(&ram->pin_WE());
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_WE());" << endl;
      }

      if (ram->pin_InClock().is_linked()) {
	    string nexus = nexus_from_link(&ram->pin_InClock());
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_InClock());" << endl;
      }

	/* Connect the address inputs... */
      for (unsigned idx = 0 ;  idx < ram->awidth() ;  idx += 1) {
	    if (! ram->pin_Address(idx).is_linked())
		  continue;

	    string nexus = nexus_from_link(&ram->pin_Address(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Address(" << idx << "));" << endl;
      }

	/* Connect the data inputs... */
      for (unsigned idx = 0 ;  idx < ram->width() ;  idx += 1) {
	    if (! ram->pin_Data(idx).is_linked())
		  continue;

	    string nexus = nexus_from_link(&ram->pin_Data(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Data(" << idx << "));" << endl;
      }

	/* Connect the data outputs... */
      for (unsigned idx = 0 ;  idx < ram->width() ;  idx += 1) {
	    if (! ram->pin_Q(idx).is_linked())
		  continue;

	    string nexus = nexus_from_link(&ram->pin_Q(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }

}

void target_vvm::logic(ostream&os, const NetLogic*gate)
{
      const string mname = mangle(gate->name());

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

      os << mname << " (" << gate->rise_time() << ");" << endl;

	/* Write the code to invoke startup for this object. */

      start_code << "      " << mname << ".start();" << endl;


	/* Setup drive strengths that are not STRONG. The gates
	   default to strong outputs, so don't generate the
	   unnecessary code to set the signal to strong. We know for
	   all these types of gates that pin(0) is the output, and the
	   vvm class takes drive0 and drive1 methods. */

      const char*str0 = vvm_val_name(verinum::V0,
				     gate->pin(0).drive0(),
				     gate->pin(0).drive1());
      if (gate->pin(0).drive0() != Link::STRONG) {
	    init_code << "      " << mname << ".drive0(" << str0 << ");"
		      << endl;
      }

      const char*str1 = vvm_val_name(verinum::V1,
				     gate->pin(0).drive0(),
				     gate->pin(0).drive1());
      if (gate->pin(0).drive1() != Link::STRONG) {
	    init_code << "      " << mname << ".drive1(" << str1 << ");"
		      << endl;
      }

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

	/* Setup drive strengths that are not STRONG. The BUFZ
	   defaults to strong outputs, so don't generate the
	   unnecessary code to set the signal to strong. We also know
	   that the vvm_bufz class takes drive0 and drive1 methods. */

      if (gate->pin(0).drive0() != Link::STRONG) {
	    const char*str = vvm_val_name(verinum::V0,
					  gate->pin(0).drive0(),
					  gate->pin(0).drive1());
	    init_code << "      " << mname << ".drive0(" << str << ");"
		      << endl;
      }

      if (gate->pin(0).drive1() != Link::STRONG) {
	    const char*str = vvm_val_name(verinum::V1,
					  gate->pin(0).drive0(),
					  gate->pin(0).drive1());
	    init_code << "      " << mname << ".drive1(" << str << ");"
		      << endl;
      }

      init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
	    mname << ");" << endl;

      nexus = nexus_from_link(&gate->pin(1));
      ncode = nexus_wire_map[nexus];

      init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
	    mname << ",0);" << endl;

}

void target_vvm::udp_comb(ostream&os, const NetUDP_COMB*gate)
{
      string mname = mangle(gate->name());
      string nexus;
      unsigned ncode;


      os << "static const char*" << mname << "_ctab =" << endl;

      string inp;
      char out;
      for (bool rc = gate->first(inp, out)
		 ;  rc ;  rc = gate->next(inp,out)) {

	    os << "    \"" << inp << out << "\"" << endl;
      }
      os << "    ;" << endl;

      os << "static vvm_udp_comb " << mname << "("
	 << (gate->pin_count()-1) << ", " << mname<<"_ctab);" << endl;

	/* Connect the output of the device... */
      nexus = nexus_from_link(&gate->pin(0));
      ncode = nexus_wire_map[nexus];
      init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
	    mname << ");" << endl;

	/* Connect the inputs of the device... */
      for (unsigned idx = 1 ;  idx < gate->pin_count() ;  idx += 1) {
	    if (! gate->pin(idx).is_linked())
		  continue;

	    nexus = nexus_from_link(&gate->pin(idx));
	    ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << "," << (idx-1) << ");" << endl;
      }

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
      assert(gate->is_sequential());
      udp_sequ_(os, gate);
}

void target_vvm::udp_sequ_(ostream&os, const NetUDP*gate)
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
 * Implement continuous assign with the force object, because they are
 * so similar. I'll be using different methods to tickle this device,
 * but it receives values the same as force.
 */
bool target_vvm::net_cassign(ostream&os, const NetCAssign*dev)
{
      string mname = mangle(dev->name());

      os << "static vvm_force " << mname << "(" << dev->pin_count()
	 << ");" << endl;

      for (unsigned idx = 0 ;  idx < dev->pin_count() ;  idx += 1) {
	    string nexus = nexus_from_link(&dev->pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << idx << ");" << endl;
      }

      return true;
}

/*
 * The NetConst is a synthetic device created to represent constant
 * values. I represent them in the output as a vvm_bufz object that
 * has its input connected to nothing but is initialized to the
 * desired constant value.
 */
void target_vvm::net_const(ostream&os, const NetConst*gate)
{
      const string mname = mangle(gate->name());

      os << "static vvm_nexus::drive_t " << mname
	 << "[" << gate->pin_count() << "];" << endl;

      for (unsigned idx = 0 ;  idx < gate->pin_count() ;  idx += 1) {
	    string nexus = nexus_from_link(&gate->pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    const char*val_str = vvm_val_name(gate->value(idx),
					      gate->pin(idx).drive0(),
					      gate->pin(idx).drive1());

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << "["<<idx<<"]);" << endl;
	    start_code << "      " << mname << "["<<idx<<"].set_value("
		       << val_str << ");" << endl;
      }

}


bool target_vvm::net_force(ostream&os, const NetForce*dev)
{
      string mname = mangle(dev->name());

      os << "static vvm_force " << mname << "(" << dev->pin_count()
	 << ");" << endl;

      for (unsigned idx = 0 ;  idx < dev->pin_count() ;  idx += 1) {
	    string nexus = nexus_from_link(&dev->pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << idx << ");" << endl;
      }

      return true;
}

void target_vvm::net_probe(ostream&os, const NetEvProbe*net)
{
      string mname = mangle(net->name());
      string mevent = mangle(net->event()->full_name());

      switch (net->edge()) {
	  case NetEvProbe::POSEDGE:
	    assert(net->pin_count() == 1);
	    os << "static vvm_posedge " << mname
	       << "(&" << mevent << ");" << endl;
	    break;

	  case NetEvProbe::NEGEDGE:
	    assert(net->pin_count() == 1);
	    os << "static vvm_negedge " << mname
	       << "(&" << mevent << ");" << endl;
	    break;


	  case NetEvProbe::ANYEDGE:
	    os << "static vvm_anyedge " << mname
	       << "(&" << mevent << ", " << net->pin_count() << ");" << endl;
	    break;
      }


	/* Connect this device as a receiver to the nexus that is my
	   source. Write the connect calls into the init code. */

      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    string nexus = nexus_from_link(&net->pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
		  mangle(net->name()) << ", " << idx << ");" << endl;
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

      os << "static bool " << thread_class_ <<
	    "_step_0_(vvm_thread*thr);" << endl;

      os << "static vvm_thread " << thread_class_ << ";" << endl;

      init_code << "      " << thread_class_ << ".step_ = &"
		<< thread_class_ << "_step_0_;" << endl;

      init_code << "      " << thread_class_ << ".thread_yield();" << endl;


      defn << "static bool " << thread_class_ << "_step_0_(vvm_thread*thr)"
	   << endl << "{" << endl;
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

	    const verinum value = rc->value();

	    if (net->bmux()) {

		    // This is a bit select. Assign the low bit of the
		    // constant to the selected bit of the lval.

		  const char*rval = vvm_val_name(value.get(0),
						 Link::STRONG,
						 Link::STRONG);

		  string bval = emit_proc_rval(defn, this, net->bmux());

		  defn << "      switch (" << bval
		       << ".as_unsigned()) {" << endl;

		  for (unsigned idx = 0; idx < net->pin_count(); idx += 1) {

			string nexus = nexus_from_link(&net->pin(idx));
			unsigned ncode = nexus_wire_map[nexus];

			defn << "      case " << idx << ":" << endl;

			defn << "        nexus_wire_table["<<ncode<<"]"
			     << ".reg_assign(" << rval << ");" << endl;
			defn << "        break;" << endl;

		  }

		  defn << "      }" << endl;
		  return;
	    }

	    for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
		  string nexus = nexus_from_link(&net->pin(idx));
		  unsigned ncode = nexus_wire_map[nexus];

		  verinum::V val = idx < value.len() 
			? value.get(idx)
			: verinum::V0;
		  const char*rval = vvm_val_name(val,
 						 Link::STRONG,
 						 Link::STRONG);

		  defn << "      nexus_wire_table[" <<ncode<< "]"
		       << ".reg_assign(" << rval << ");" << endl;
	    }
	    return;
      }


      string rval;


	/* Handle another special case, that of an r-value that is a
	   simple identifier. In this case we don't need to generate
	   the vvm_bitset_t but can pull the result directly out of
	   the identifier memory. It is OK to turn the r-value string
	   into a simple vpip_bit_t array (the .bits member of the
	   signal) because we know that we will only be using the []
	   operator on it. */

      if (const NetESignal*rs = dynamic_cast<const NetESignal*>(net->rval())) {

	    if (net->pin_count() > rs->pin_count()) {
		  rval = emit_proc_rval(defn, this, net->rval());

	    } else {
		  assert((net->pin_count() <= rs->pin_count())
			 || (net->bmux() && (rs->pin_count() >= 1)));
		  rval = mangle(rs->name()) + ".bits";
	    }

      } else {

	    rval = emit_proc_rval(defn, this, net->rval());
      }


      defn << "      // " << net->get_line() << ": " << endl;


	/* Now, if there is a mux on the l-value, generate a code to
	   assign a single bit to one of the bits of the
	   l-value. Otherwise, generate code for a complete
	   assignment. */

      if (net->bmux()) {

	      // This is a bit select. Assign the low bit of the rval
	      // to the selected bit of the lval.
	    string bval = emit_proc_rval(defn, this, net->bmux());

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
	    unsigned min_count = net->pin_count();
	    if (net->rval()->expr_width() < min_count)
		  min_count = net->rval()->expr_width();

	    for (unsigned idx = 0 ;  idx < min_count ;  idx += 1) {
		  string nexus = nexus_from_link(&net->pin(idx));
		  unsigned ncode = nexus_wire_map[nexus];
		  defn << "      nexus_wire_table["<<ncode<<"].reg_assign("
		       << rval << "[" << idx << "]);" << endl;
	    }

	    for (unsigned idx = min_count; idx < net->pin_count(); idx += 1) {
		  string nexus = nexus_from_link(&net->pin(idx));
		  unsigned ncode = nexus_wire_map[nexus];
		  defn << "      nexus_wire_table["<<ncode<<"]"
		       << ".reg_assign(St0);" << endl;
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
	/* make a temporary to reference the index signal. */
      string index = emit_proc_rval(defn, this, amem->index());

	/* Evaluate the rval that gets written into the memory word. */
      string rval = emit_proc_rval(defn, this, amem->rval());


      const NetMemory*mem = amem->memory();
      assert(mem->width() <= amem->rval()->expr_width());

      defn << "      /* " << amem->get_line() << " */" << endl;

	/* Set the indexed word from the rval. Note that this
	   assignment will work even if the rval is too wide, because
	   the set_word method takes only the low bits of the width of
	   the memory. */

      defn << "      " << mangle(mem->name()) <<
	    ".set_word(" << index << ".as_unsigned(), " <<
	    rval << ");" << endl;

}

void target_vvm::proc_assign_nb(ostream&os, const NetAssignNB*net)
{
      string rval = emit_proc_rval(defn, this, net->rval());
      const unsigned long delay = net->rise_time();

      if (net->bmux()) {
	      /* If the l-value has a bit select, set the output bit
		 to only the desired bit. Evaluate the index and use
		 that to drive a switch statement.

		 XXXX I'm not fully satisfied with this, I might like
		 better generating a demux device and doing the assign
		 to the device input. Food for thought. */

	    string bval = emit_proc_rval(defn, this, net->bmux());
	    defn << "      switch (" << bval << ".as_unsigned()) {" << endl;

	    for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
		  string nexus = nexus_from_link(&net->pin(idx));
		  unsigned ncode = nexus_wire_map[nexus];

		  defn << "      case " << idx << ":" << endl;
		  defn << "        vvm_delayed_assign(nexus_wire_table["
		       << ncode << "], " << rval << "[0], " << delay << ");"
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
	/* make a temporary to reference the index signal. */
      string index = emit_proc_rval(defn, this, amem->index());


	/* Evaluate the rval that gets written into the memory word. */
      string rval = emit_proc_rval(defn, this, amem->rval());

      const NetMemory*mem = amem->memory();

      defn << "      /* " << amem->get_line() << " */" << endl;

      assert(mem->width() <= amem->rval()->expr_width());

      defn << "      (new vvm_memory_t<" << mem->width() << ","
	   << mem->count() << ">::assign_nb(" << mangle(mem->name())
	   << ", " << index << ".as_unsigned(), " << rval <<
	    ")) -> schedule();" << endl;
}

bool target_vvm::proc_block(ostream&os, const NetBlock*net)
{
      if (net->type() == NetBlock::SEQU) {
	    net->emit_recurse(os, this);
	    return true;
      }

      unsigned exit_step = thread_step_ + 1;

      thread_step_ += 1;

      const NetProc*cur;
      unsigned cnt = 0;
      unsigned idx;

	// Declare the exit step...

      os << "static bool " << thread_class_ << "_step_" << exit_step
	 << "_(vvm_thread*thr);" << endl;


	// Declare the first steps for all the threads to be created,
	// and count those threads while I'm at it.

      for (cur = net->proc_first() ;  cur ;  cur = net->proc_next(cur)) {
	    cnt += 1;
	    os << "static bool " << thread_class_ << "_step_"
	       << (exit_step+cnt) << "_(vvm_thread*thr);" << endl;
      }

      thread_step_ += cnt;

	// Write the code to start all the threads, then pause the
	// current thread.

      defn << "      thr->callee_ = new vvm_thread["<<cnt<<"];" << endl;
      defn << "      thr->ncallee_ = " << cnt << ";" << endl;

      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    defn << "      thr->callee_["<<idx<<"].back_ = thr;" << endl;
	    defn << "      thr->callee_["<<idx<<"].step_ = &"
		 << thread_class_ << "_step_" << (exit_step+idx+1)
		 << "_;" << endl;
	    defn << "      thr->callee_["<<idx<<"].thread_yield();" << endl;
      }

      defn << "      thr->step_ = &" << thread_class_ << "_step_"
	   << exit_step << "_;" << endl;
      defn << "      return false;" << endl;
      defn << "}" << endl;

	// Generate the thread steps. At the end of the thread proc,
	// write code to manage the join.

      for (idx = 0, cur = net->proc_first()
		 ;  cur ;  idx +=1, cur = net->proc_next(cur)) {

	    defn << "static bool " << thread_class_ << "_step_"
		 << (exit_step+idx+1) << "_(vvm_thread*thr) {" << endl;

	    cur->emit_proc(os, this);

	    defn << "      thr->back_->ncallee_ -= 1;" << endl;
	    defn << "      if (thr->back_->ncallee_ == 0)" << endl;
	    defn << "          thr->back_->thread_yield();" << endl;
	    defn << "      return false;" << endl;
	    defn << "}" << endl;
      }

	// Finally, start the exit step.

      defn << "static bool " << thread_class_ << "_step_" << exit_step
	   << "_(vvm_thread*thr) {" << endl;

      defn << "      delete[]thr->callee_;" << endl;
      defn << "      thr->callee_ = 0;" << endl;

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
      string expr = emit_proc_rval(defn, this, net->expr());

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
	    string guard = emit_proc_rval(defn, this, net->expr(idx));

	    defn << "      if (B_IS1(" << test_func << "(" << guard << ","
	       << expr << "))) {" << endl;
	    defn << "          thr->step_ = &" << thread_class_ <<
		  "_step_" << thread_step_ << "_;" << endl;
	    defn << "          return true;" << endl;
	    defn << "      }" << endl;
      }

	/* If none of the above tests pass, then branch to the default
	   step (or the exit step if there is no default.) */
      if (default_idx < net->nitems()) {
	    thread_step_ += 1;

	    defn << "      /* default : */" << endl;
	    defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
		  thread_step_ << "_;" << endl;

      } else {
	    defn << "      /* no default ... fall out of case. */" << endl;
	    defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
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

	    os << "static bool " << thread_class_ << "_step_"
		 << step_num << "_(vvm_thread*thr);" << endl;

	    defn << "static bool " << thread_class_ << "_step_"
		 << step_num << "_(vvm_thread*thr) {" << endl;
	    if (net->stat(idx))
		  net->stat(idx)->emit_proc(os, this);
	    defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
		  exit_step << "_;" << endl;
	    defn << "      return true;" << endl;
	    defn << "}" << endl;
      }

	/* If there is a default case, generate the default step. */
      if (default_idx < net->nitems()) {
	    step_num += 1;

	    os << "static bool " << thread_class_ << "_step_"
	       << step_num << "_(vvm_thread*thr);" << endl;

	    defn << "static bool " << thread_class_ << "_step_"
		 << step_num << "_(vvm_thread*thr) {" << endl;
	    if (net->stat(default_idx))
		  net->stat(default_idx)->emit_proc(os, this);
	    defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
		  exit_step << "_;" << endl;
	    defn << "      return true;" << endl;
	    defn << "}" << endl;
      }

	/* Finally, start the exit step. */

      os << "static bool " << thread_class_ << "_step_"
	 << exit_step << "_(vvm_thread*thr);" << endl;

      defn << "static bool " << thread_class_ << "_step_"
	   << exit_step << "_(vvm_thread*thr) {" << endl;
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

      string expr = emit_proc_rval(defn, this, net->expr());

      unsigned default_idx = net->nitems();
      for (unsigned idx = 0 ;  idx < net->nitems() ;  idx += 1) {

	      // don't emit the default case here. Save it for the
	      // last else clause.
	    if (net->expr(idx) == 0) {
		  default_idx = idx;
		  continue;
	    }

	    string guard = emit_proc_rval(defn, this, net->expr(idx));

	    defn << "      if (B_IS1(" << test_func << "(" <<
		  guard << "," << expr << "))) {" << endl;
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

bool target_vvm::proc_cassign(ostream&os, const NetCAssign*dev)
{
      const string mname = mangle(dev->name());

      for (unsigned idx = 0 ;  idx < dev->pin_count() ;  idx += 1) {
	    string nexus = nexus_from_link(&dev->lval_pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    defn << "      " << mname << ".assign("<<idx<<", "
		 << "nexus_wire_table+"<<ncode << ");" << endl;
      }

      return true;
}

void target_vvm::proc_condit(ostream&os, const NetCondit*net)
{
      if (function_def_flag_) {
	    proc_condit_fun(os, net);
	    return;
      }

      string expr = emit_proc_rval(defn, this, net->expr());

      unsigned if_step   = ++thread_step_;
      unsigned else_step = ++thread_step_;
      unsigned out_step  = ++thread_step_;

	/* Declare new steps that I am going to create. */

      os << "static bool " << thread_class_ << "_step_"
	 << if_step << "_(vvm_thread*thr);" << endl;

      os << "static bool " << thread_class_ << "_step_"
	 << else_step << "_(vvm_thread*thr);" << endl;

      os << "static bool " << thread_class_ << "_step_"
	 << out_step << "_(vvm_thread*thr);" << endl;

      defn << "      if (B_IS1(" << expr << "[0]))" << endl;
      defn << "        thr->step_ = &" << thread_class_ << "_step_" <<
	    if_step << "_;" << endl;
      defn << "      else" << endl;
      defn << "        thr->step_ = &" << thread_class_ << "_step_" <<
	    else_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      defn << "static bool " << thread_class_ << "_step_" << if_step <<
	    "_(vvm_thread*thr) {" << endl;
      net->emit_recurse_if(os, this);
      defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
	    out_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      defn << "static bool " << thread_class_ << "_step_" << else_step <<
	    "_(vvm_thread*thr) {" << endl;
      net->emit_recurse_else(os, this);
      defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
	    out_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      defn << "static bool " << thread_class_ << "_step_" << out_step <<
	    "_(vvm_thread*thr) {" << endl;
}

void target_vvm::proc_condit_fun(ostream&os, const NetCondit*net)
{
      string expr = emit_proc_rval(defn, this, net->expr());

      defn << "      // " << net->get_line() << ": conditional (if-else)"
	   << endl;
      defn << "      if (B_IS1(" << expr << "[0])) {" << endl;
      net->emit_recurse_if(os, this);
      defn << "      } else {" << endl;
      net->emit_recurse_else(os, this);
      defn << "      }" << endl;
}

bool target_vvm::proc_deassign(ostream&os, const NetDeassign*dev)
{
      const NetNet*lval = dev->lval();
      for (unsigned idx = 0 ;  idx < lval->pin_count() ;  idx += 1) {
	    string nexus = nexus_from_link(&lval->pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    defn << "      nexus_wire_table["<<ncode<<"].deassign();"
		 << endl;
      }

      return true;
}

bool target_vvm::proc_force(ostream&os, const NetForce*dev)
{
      const string mname = mangle(dev->name());

      for (unsigned idx = 0 ;  idx < dev->pin_count() ;  idx += 1) {
	    string nexus = nexus_from_link(&dev->lval_pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    defn << "      " << mname << ".force("<<idx<<", "
		 << "nexus_wire_table+"<<ncode << ");" << endl;
      }

      return true;
}

/*
 * The forever loop is implemented by starting a basic block, handing
 * the statement, and putting in a goto to the beginning of the block.
 * This is arranged in vvm by starting a step that is the top of the
 * loop, elaborating the contents of the loop, then returning to the
 * top step after the loop contents.
 */
void target_vvm::proc_forever(ostream&os, const NetForever*net)
{
      unsigned top_step = ++thread_step_;
      unsigned out_step = ++thread_step_;

      defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
	    top_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      os << "static bool " << thread_class_ << "_step_"
	 << top_step << "_(vvm_thread*thr);" << endl;

      defn << "static bool " << thread_class_ << "_step_"
	   << top_step << "_(vvm_thread*thr) {" << endl;

      net->emit_recurse(os, this);
      defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
	    top_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

	/* Generate a loop out step to catch unreachable stuff afer
	   the loop. */

      os << "static bool " << thread_class_ << "_step_"
	 << out_step << "_(vvm_thread*thr);" << endl;
     
      defn << "static bool " << thread_class_ << "_step_"
	   << out_step << "_(vvm_thread*thr) {" << endl;
}

bool target_vvm::proc_release(ostream&os, const NetRelease*dev)
{
      const NetNet*lval = dev->lval();
      for (unsigned idx = 0 ;  idx < lval->pin_count() ;  idx += 1) {
	    string nexus = nexus_from_link(&lval->pin(idx));
	    unsigned ncode = nexus_wire_map[nexus];

	    defn << "      nexus_wire_table["<<ncode<<"].release();"
		 << endl;
      }

      return true;
}

void target_vvm::proc_repeat(ostream&os, const NetRepeat*net)
{
      unsigned top_step = ++thread_step_;
      unsigned out_step = ++thread_step_;

	/* Emit the index expression value and convert it into a
	   native number. If the expression is a constant, then do it
	   directly. Otherwise, generate the code to do it. */
      string expr;
      if (const NetEConst*val = dynamic_cast<const NetEConst*>(net->expr())) {
	    char tmp[64];
	    sprintf(tmp, "%lu", val->value().as_ulong());
	    expr = tmp;

      } else {
	    expr = emit_proc_rval(defn, this, net->expr());
	    expr = expr + ".as_unsigned()";
      }

	/* Declare a variable to use as a loop index. */
      os << "static unsigned " << thread_class_ << "_step_"
	 << top_step << "_idx_;" << endl;

	/* Declare the top step. */
      os << "static bool " << thread_class_ << "_step_"
	 << top_step << "_(vvm_thread*thr);" << endl;

	/* Declare the exit step. */
      os << "static bool " << thread_class_ << "_step_"
	 << out_step << "_(vvm_thread*thr);" << endl;


      defn << "      " << thread_class_ << "_step_"
	   << top_step << "_idx_ = " << expr << ";" << endl;
      defn << "      thr->step_ = &" << thread_class_ << "_step_"
	   << top_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      defn << "static bool " << thread_class_ << "_step_"
	   << top_step << "_(vvm_thread*thr) {" << endl;

      defn << "      if (" << thread_class_ << "_step_"
	   << top_step << "_idx_ == 0) {" << endl;
      defn << "        thr->step_ = &" << thread_class_ << "_step_"
	   << out_step << "_;" << endl;
      defn << "        return true;" << endl;
      defn << "      }" << endl;
      defn << "      " << thread_class_ << "_step_"
	   << top_step << "_idx_ -= 1;" << endl;

      net->emit_recurse(os,this);

      defn << "      thr->step_ = &" << thread_class_ << "_step_"
	   << top_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;


      defn << "static bool " << thread_class_ << "_step_"
	   << out_step << "_(vvm_thread*thr) {" << endl;
}

/*
 * Calls to system tasks are done here. We know that this is a system
 * task and that I need to generate an external call. Calls to user
 * defined tasks are handled elsewhere.
 */
void target_vvm::proc_stask(ostream&os, const NetSTask*net)
{
	/* Handle the special case of a system task without any
	   parameters. we don't need a parameter array for this. */

      if (net->nparms() == 0) {
	    defn << "      vpip_calltask(\"" << net->name()
		 << "\", 0, 0);" << endl;
	    defn << "      if (vpip_finished()) return false;" << endl;
	    return;
      }

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

bool target_vvm::proc_trigger(ostream&os, const NetEvTrig*trig)
{
      const NetEvent*ev = trig->event();
      assert(ev);

      string ename = mangle(ev->full_name());

      defn << "      " << ename << ".wakeup(); // "
	   << trig->get_line() << ": -> " << ev->full_name() << endl;

      return true;
}

void target_vvm::proc_utask(ostream&os, const NetUTask*net)
{
      unsigned out_step = ++thread_step_;
      const string name = mangle(net->name());

      os << "static bool " << thread_class_ << "_step_"
	 << out_step << "_(vvm_thread*thr);" << endl;

      defn << "      assert(thr->callee_ == 0);" << endl;
      defn << "      thr->callee_ = new vvm_thread;" << endl;
      defn << "      thr->callee_->back_ = thr;" << endl;
      defn << "      thr->callee_->step_ = &" << name << "_step_0_;" << endl;
      defn << "      thr->callee_->thread_yield();" << endl;
      defn << "      thr->step_ = &" << thread_class_ << "_step_"
	   << out_step << "_;" << endl;
      defn << "      return false;" << endl;
      defn << "}" << endl;

      defn << "static bool " << thread_class_ << "_step_"
	   << out_step << "_(vvm_thread*thr) {" << endl;
      defn << "      delete thr->callee_;" << endl;
      defn << "      thr->callee_ = 0;" << endl;
}

bool target_vvm::proc_wait(ostream&os, const NetEvWait*wait)
{
      unsigned out_step = ++thread_step_;

      defn << "      thr->step_ = &" << thread_class_ << "_step_"
	   << out_step << "_;" << endl;

      if (wait->nevents() == 1) {
		  const NetEvent*ev = wait->event(0);
		  assert(ev);
		  string ename = mangle(ev->full_name());
		  defn << "      " << ename << ".wait(thr); // "
		       << wait->get_line() << ": @" << ev->full_name()
		       << "..." << endl;

      } else {
	      /* If there are many events to wait for, generate a
		 selector that is a vvm_sync that I chain to the
		 source vvm_sync objects. Then, wait on the selector
		 object instead. */
	    unsigned id = selector_counter++;
	    os << "static vvm_sync selector_" << id << ";" << endl;

	    for (unsigned idx = 0 ;  idx < wait->nevents() ;  idx+= 1) {
		  const NetEvent*ev = wait->event(idx);
		  assert(ev);
		  string ename = mangle(ev->full_name());
		  init_code << "      selector_" << id
			    << ".chain_sync(&" << ename << "); // "
			    << wait->get_line() << ": @" << ev->full_name()
			    << "..." << endl;
	    }

	    defn << "      selector_" << id << ".wait(thr);"
		 << endl;
      }

      defn << "      return false;" << endl;
      defn << "}" << endl;

      os << "static bool " << thread_class_ << "_step_" << out_step
	 << "_(vvm_thread*thr);" << endl;

      defn << "bool " << thread_class_ << "_step_" << out_step
	   << "_(vvm_thread*thr) {" << endl;

      return wait->emit_recurse(os, this);
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

      os << "static bool " << thread_class_ << "_step_"
	 << head_step << "_(vvm_thread*thr);" << endl;

      os << "static bool " << thread_class_ << "_step_"
	 << out_step << "_(vvm_thread*thr);" << endl;

      defn << "      thr->step_ = &" << thread_class_ << "_step_"
	   << head_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      defn << "// " << net->expr()->get_line() <<
	    ": top of while condition." << endl;
      defn << "static bool " << thread_class_ << "_step_"
	   << head_step << "_(vvm_thread*thr) {" << endl;

      string expr = emit_proc_rval(defn, this, net->expr());

      defn << "// " << net->expr()->get_line() <<
	    ": test while condition." << endl;
      defn << "      if (!B_IS1(" << expr << "[0])) {" << endl;
      defn << "          thr->step_ = &" << thread_class_ << "_step_"
	   << out_step << "_;" << endl;
      defn << "          return true;" << endl;
      defn << "      }" << endl;

      net->emit_proc_recurse(os, this);

      defn << "// " << net->expr()->get_line() <<
	    ": end of while loop." << endl;
      defn << "      thr->step_ = &" << thread_class_ << "_step_"
	   << head_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      defn << "static bool " << thread_class_ << "_step_"
	   << out_step << "_(vvm_thread*thr) {" << endl;
}


/*
 * A delay suspends the thread for a period of time.
 */
void target_vvm::proc_delay(ostream&os, const NetPDelay*proc)
{
      thread_step_ += 1;
      defn << "      thr->step_ = &" << thread_class_ << "_step_"
	   << thread_step_ << "_;" << endl;
      defn << "      thr->thread_yield(" << proc->delay() << ");" << endl;
      defn << "      return false;" << endl;
      defn << "}" << endl;

      os << "static bool " << thread_class_ << "_step_"
	 << thread_step_ << "_(vvm_thread*thr);" << endl;

      defn << "static bool " << thread_class_ << "_step_" << thread_step_
	   << "_(vvm_thread*thr) {" << endl;

      proc->emit_proc_recurse(os, this);
}

void target_vvm::end_process(ostream&os, const NetProcTop*proc)
{
      if (proc->type() == NetProcTop::KALWAYS) {
	    defn << "      thr->step_ = &" << thread_class_ << "_step_0_;"
	       << endl;
	    defn << "      return true;" << endl;
      } else {
	    defn << "      thr->step_ = 0;" << endl;
	    defn << "      return false;" << endl;
      }

      defn << "}" << endl;
}


static target_vvm target_vvm_obj;

extern const struct target tgt_vvm = {
      "vvm",
      &target_vvm_obj
};
/*
 * $Log: t-vvm.cc,v $
 * Revision 1.157  2000/06/13 03:24:48  steve
 *  Index in memory assign should be a NetExpr.
 *
 * Revision 1.156  2000/06/06 02:32:45  steve
 *  Expand constants in its special case assignment. (Stephan Boettcher)
 *
 * Revision 1.155  2000/06/03 02:13:43  steve
 *  Do not attach temporaries to scopes.
 *
 * Revision 1.154  2000/05/31 03:52:07  steve
 *  The NetESignal shortcut cannot expand an rvalue.
 *
 * Revision 1.153  2000/05/25 06:02:12  steve
 *  Emin init code only once per nexus.
 *
 * Revision 1.152  2000/05/25 01:45:35  steve
 *  Optimize assignment from signals.
 *
 * Revision 1.151  2000/05/20 02:48:51  steve
 *  Add vpi numbers to the bits table.
 *
 * Revision 1.150  2000/05/20 02:26:23  steve
 *  Combine constants into a bit table.
 *
 * Revision 1.149  2000/05/11 23:37:27  steve
 *  Add support for procedural continuous assignment.
 *
 * Revision 1.148  2000/05/09 21:16:35  steve
 *  Give strengths to logic and bufz devices.
 *
 * Revision 1.147  2000/05/07 21:17:21  steve
 *  non-blocking assignment to a bit select.
 *
 * Revision 1.146  2000/05/07 19:40:26  steve
 *  Fix connection of Direction of LMP_CLSHIFT
 *  to constant values. Remember to add a signal
 *  to the nexus and connect the receiver in vvm.
 *
 * Revision 1.145  2000/05/07 18:20:07  steve
 *  Import MCD support from Stephen Tell, and add
 *  system function parameter support to the IVL core.
 *
 * Revision 1.144  2000/05/07 04:37:56  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.143  2000/05/04 03:37:59  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.142  2000/05/02 00:58:12  steve
 *  Move signal tables to the NetScope class.
 *
 * Revision 1.141  2000/04/28 18:43:23  steve
 *  integer division in expressions properly get width.
 *
 * Revision 1.140  2000/04/26 18:35:11  steve
 *  Handle assigning small values to big registers.
 *
 * Revision 1.139  2000/04/23 03:45:24  steve
 *  Add support for the procedural release statement.
 *
 * Revision 1.138  2000/04/22 04:20:19  steve
 *  Add support for force assignment.
 *
 * Revision 1.137  2000/04/15 19:51:30  steve
 *  fork-join support in vvm.
 *
 * Revision 1.136  2000/04/15 02:25:32  steve
 *  Support chained events.
 *
 * Revision 1.135  2000/04/14 23:31:53  steve
 *  No more class derivation from vvm_thread.
 *
 * Revision 1.134  2000/04/12 20:02:53  steve
 *  Finally remove the NetNEvent and NetPEvent classes,
 *  Get synthesis working with the NetEvWait class,
 *  and get started supporting multiple events in a
 *  wait in vvm.
 *
 * Revision 1.133  2000/04/12 04:23:58  steve
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
 *
 * Revision 1.132  2000/04/10 05:26:06  steve
 *  All events now use the NetEvent class.
 *
 * Revision 1.131  2000/04/09 16:55:42  steve
 *  Donot create tables that have no entries.
 *
 * Revision 1.130  2000/04/04 03:20:15  steve
 *  Simulate named event trigger and waits.
 *
 * Revision 1.129  2000/04/02 04:26:07  steve
 *  Remove the useless sref template.
 *
 * Revision 1.128  2000/04/01 21:40:23  steve
 *  Add support for integer division.
 *
 * Revision 1.127  2000/03/29 04:37:11  steve
 *  New and improved combinational primitives.
 *
 * Revision 1.126  2000/03/26 16:28:31  steve
 *  vvm_bitset_t is no longer a template.
 *
 * Revision 1.125  2000/03/25 05:02:24  steve
 *  signal bits are referenced at run time by the vpiSignal struct.
 *
 * Revision 1.124  2000/03/25 02:43:56  steve
 *  Remove all remain vvm_bitset_t return values,
 *  and disallow vvm_bitset_t copying.
 *
 * Revision 1.123  2000/03/24 03:47:01  steve
 *  Update vvm_ram_dq to nexus style.
 *
 * Revision 1.122  2000/03/24 02:43:36  steve
 *  vvm_unop and vvm_binop pass result by reference
 *  instead of returning a value.
 *
 * Revision 1.121  2000/03/23 03:24:39  steve
 *  Do not create 0 length parameters to system tasks.
 *
 * Revision 1.120  2000/03/22 04:26:40  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.119  2000/03/20 17:40:33  steve
 *  Do not link adder pins that ar unconnected.
 */

