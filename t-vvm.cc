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
#ident "$Id: t-vvm.cc,v 1.206 2001/04/22 23:09:46 steve Exp $"
#endif

# include  <iostream>
# include  <fstream>
# include  <strstream>
# include  <iomanip>
# include  <string>
# include  <typeinfo>
# include  <unistd.h>
# include  <stdio.h>
# include  "netlist.h"
# include  "netmisc.h"
# include  "target.h"

  // Comparison for use in sorting algorithms.

struct less_verinum {
      bool operator() (const verinum&left, const verinum&right) const
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

    public:
      target_vvm();
      ~target_vvm();

      virtual bool start_design(const Design*);
      virtual void scope(const NetScope*);
      virtual void event(const NetEvent*);
      virtual void signal(const NetNet*);
      virtual void memory(const NetMemory*);
      virtual void task_def(const NetScope*);
      virtual void func_def(const NetScope*);

      virtual void lpm_add_sub(const NetAddSub*);
      virtual void lpm_clshift(const NetCLShift*);
      virtual void lpm_compare(const NetCompare*);
      virtual void lpm_divide(const NetDivide*);
      virtual void lpm_modulo(const NetModulo*);
      virtual void lpm_ff(const NetFF*);
      virtual void lpm_mult(const NetMult*);
      virtual void lpm_mux(const NetMux*);
      virtual void lpm_ram_dq(const NetRamDq*);

      virtual void logic(const NetLogic*);
      virtual bool bufz(const NetBUFZ*);
      virtual void udp(const NetUDP*);
      virtual void net_assign(const NetAssign_*) { }
      virtual void net_case_cmp(const NetCaseCmp*);
      virtual bool net_cassign(const NetCAssign*);
      virtual bool net_const(const NetConst*);
      virtual bool net_force(const NetForce*);
      virtual void net_probe(const NetEvProbe*);
      virtual bool process(const NetProcTop*);
      virtual void proc_assign(const NetAssign*);
              void proc_assign_rval(const NetAssign_*, const NetEConst*,
				    unsigned off);
              void proc_assign_rval(const NetAssign_*, const string&,
				    unsigned wid, unsigned off);
      virtual void proc_assign_mem(const NetAssignMem*);
      virtual void proc_assign_nb(const NetAssignNB*);
              void proc_assign_nb_rval(const NetAssign_*, const NetEConst*,
				       unsigned off);
              void proc_assign_nb_rval(const NetAssign_*, const string&,
				       unsigned wid, unsigned off);
      virtual void proc_assign_mem_nb(const NetAssignMemNB*);
      virtual bool proc_block(const NetBlock*);
      virtual void proc_case(const NetCase*net);
              void proc_case_fun(ostream&os, const NetCase*net);
      virtual bool proc_cassign(const NetCAssign*);
      virtual void proc_condit(const NetCondit*);
              void proc_condit_fun(ostream&os, const NetCondit*);
      virtual bool proc_deassign(const NetDeassign*);
      virtual bool proc_delay(const NetPDelay*);
      virtual bool proc_force(const NetForce*);
      virtual void proc_forever(const NetForever*);
      virtual bool proc_release(const NetRelease*);
      virtual void proc_repeat(const NetRepeat*);
      virtual void proc_stask(const NetSTask*);
      virtual bool proc_trigger(const NetEvTrig*);
      virtual void proc_utask( const NetUTask*);
      virtual bool proc_wait( const NetEvWait*);
      virtual void proc_while(const NetWhile*);
      virtual int  end_design(const Design*);

      void start_process(ostream&os, const NetProcTop*);
      void end_process(ostream&os, const NetProcTop*);


      NumberTable bits_table;

      ofstream out;

	// Method definitions go into this file.
      char*defn_name;
      ofstream defn;

      char*init_code_name;
      ofstream init_code;

      char *start_code_name;
      ofstream start_code;

      unsigned process_counter;
      unsigned thread_step_;

	// String constants that are made into vpiHandles have their
	// handle name mapped by this.
      map<string,unsigned>string_constants;
      unsigned string_counter;

	// Number constants accessed by vpiHandles are mapped by this.
      map<verinum,unsigned,less_verinum>number_constants;
      unsigned number_counter;

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

	// These methods are use to help prefent duplicate printouts
	// of things that may be scanned multiple times.
      map<string,bool>esignal_printed_flag;
      map<string,bool>pevent_printed_flag;

      unsigned signal_bit_counter;
      unsigned signal_counter;

    public:
      map<string,unsigned>nexus_wire_map;
      unsigned nexus_wire_counter;

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
      explicit vvm_proc_rval(target_vvm*t)
      : result(""), tgt_(t) { }

      string result;

    private:
      target_vvm*tgt_;

    private:
      virtual void expr_const(const NetEConst*);
      virtual void expr_concat(const NetEConcat*);
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
      explicit vvm_parm_rval(target_vvm*t)
      : result(""), tgt_(t) { }

      string result;

    private:
      virtual void expr_binary(const NetEBinary*);
      virtual void expr_concat(const NetEConcat*);
      virtual void expr_const(const NetEConst*);
      virtual void expr_memory(const NetEMemory*);
      virtual void expr_scope(const NetEScope*);
      virtual void expr_sfunc(const NetESFunc*);
      virtual void expr_signal(const NetESignal*);
      virtual void expr_ufunc(const NetEUFunc*);
      virtual void expr_unary(const NetEUnary*);

    private:
      target_vvm*tgt_;

      void expr_default_(const NetExpr*);
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

      tgt_->defn << "      vpip_bit_t " << tname << "_bits["
		 << expr->expr_width() << "];" << endl;
      tgt_->defn << "      vvm_bitset_t " << tname << "(" << tname << "_bits, "
		 << expr->expr_width() << ");" << endl;

      unsigned pos = 0;
      for (unsigned rept = 0 ;  rept < expr->repeat() ;  rept += 1)
	    for (unsigned idx = 0 ;  idx < expr->nparms() ;  idx += 1) {

		  NetExpr*pp = expr->parm(expr->nparms() - idx - 1);
		  pp->expr_scan(this);

		  for (unsigned bit = 0 ; bit < pp->expr_width() ; bit += 1) {
			tgt_->defn << "      " << tname << "[" << pos
				   <<"] = " << result << "[" << bit << "];"
				   << endl;
			pos+= 1;
		  }
		  assert(pos <= expr->expr_width());
	    }

	/* Check that the positions came out to the right number of
	   bits. */
      if (pos != expr->expr_width()) {
	    tgt_->defn << "#error \"" << expr->get_line() << ": vvm error: "
		       << "width is " << expr->expr_width() << ", but I count "
		       << pos << " bits.\"" << endl;
      }

      result = tname;
}

void vvm_proc_rval::expr_const(const NetEConst*expr)
{
      string tname = make_temp();

      unsigned number_off = tgt_->bits_table.position(expr->value());
      tgt_->defn << "      vvm_bitset_t " << tname << "(const_bits_table+"
		 << number_off << ", " << expr->expr_width() << ");" << endl;

      result = tname;
}

/*
 * a bitset rval that is a memory reference.
 */
void vvm_proc_rval::expr_memory(const NetEMemory*mem)
{
	/* Make a temporary to hold the word from the memory. */
      const string tname = make_temp();
      tgt_->defn << "      vpip_bit_t " << tname << "_bits["
		 << mem->expr_width() << "];" << endl;
      tgt_->defn << "      vvm_bitset_t " << tname << "(" << tname << "_bits, "
		 << mem->expr_width() << ");" << endl;

      const string mname = mangle(mem->name());

	/* Evaluate the memory index */
      assert(mem->index());
      mem->index()->expr_scan(this);


	/* Write code to use the calculated index to get the word from
	   the memory into the temporary we created earlier. */

      tgt_->defn << "      " << mname << ".get_word("
		 << result << ".as_unsigned(), " << tname << ");" << endl;

      result = tname;
}

void vvm_proc_rval::expr_sfunc(const NetESFunc*fun)
{
      tgt_->defn << "      // " << fun->get_line() << endl;

      const string retval = make_temp();
      const unsigned retwid = fun->expr_width();

	/* Make any parameters that might be needed to be passed to
	   the function. */

      const string parmtab = make_temp();
      if (fun->nparms() > 0) {
	    tgt_->defn << "      vpiHandle " << parmtab
		       << "["<<fun->nparms()<<"];" << endl;

	    for (unsigned idx = 0 ;  idx < fun->nparms() ;  idx += 1) {
		  vvm_parm_rval scan(tgt_);
		  fun->parm(idx)->expr_scan(&scan);

		  tgt_->defn << "      " << parmtab <<"["<<idx<<"] = "
			     << scan.result << ";" << endl;
	    }
      }

	/* Draw the call to the function. Create a vpip_bit_t array to
	   receive the return value, and make it into a vvm_bitset_t
	   when the call returns. */

      tgt_->defn << "      vpip_bit_t " << retval << "_bits["<<retwid<<"];"
		 << endl;

      tgt_->defn << "      vpip_callfunc(\"" << stresc(fun->name()) << "\", "
		 << retwid << ", " << retval<<"_bits";

      if (fun->nparms() == 0)
	    tgt_->defn << ", 0, 0";
      else
	    tgt_->defn << ", " << fun->nparms() << ", " << parmtab;

      tgt_->defn << ");" << endl;

      tgt_->defn << "      vvm_bitset_t " << retval << "(" << retval<<"_bits, "
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

      tgt_->defn << "      vvm_bitset_t " << tname << "("
		 << mangle(expr->name()) << ".bits, "
		 << expr->expr_width() << ");" << endl;

      result = tname;
}

void vvm_proc_rval::expr_subsignal(const NetESubSignal*sig)
{
      string idx = make_temp();
      string val = make_temp();
      if (const NetEConst*cp = dynamic_cast<const NetEConst*>(sig->index())) {
	    tgt_->defn << "      const unsigned " << idx
		       << " = " << cp->value().as_ulong() << ";" << endl;

      } else {
	    sig->index()->expr_scan(this);
	    tgt_->defn << "      const unsigned " << idx
		       << " = " << result << ".as_unsigned();" << endl;
      }

	/* Get the bit select of a signal by making a vvm_bitset_t
	   object that refers to the single bit within the signal that
	   is of interest. */

      tgt_->defn << "      vvm_bitset_t " << val << "("
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

      tgt_->defn << "      vpip_bit_t " << result << "_bits["
		 << expr->expr_width() << "];" << endl;
      tgt_->defn << "      vvm_bitset_t " << result << "(" << result<<"_bits, "
		 << expr->expr_width() << ");" << endl;

      tgt_->defn << "      vvm_ternary(" << result << ", " << cond_val<<"[0], "
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
      const NetFuncDef*def = expr->func()->func_def();
      const unsigned pcnt = expr->parm_count();
      assert(pcnt == (def->port_count()-1));

	/* Scan the parameter expressions, and assign the values to
	   the parameter port register. */
      for (unsigned idx = 0 ;  idx < pcnt ;  idx += 1) {
	    assert(expr->parm(idx));

	    const NetNet*pnet = def->port(idx+1);
	    assert(pnet);

	    expr->parm(idx)->expr_scan(this);
	    tgt_->defn << "      // " << pnet->name() << " == "
		       << result << endl;

	    unsigned wid = expr->parm(idx)->expr_width();
	    if (pnet->pin_count() < wid)
		  wid = pnet->pin_count();

	    string bname = mangle(def->port(idx+1)->name());
	    for (unsigned bit = 0 ;  bit < wid ;  bit += 1) {

		  string nexus = pnet->pin(bit).nexus()->name();
		  unsigned ncode = tgt_->nexus_wire_map[nexus];

		  tgt_->defn << "      nexus_wire_table["<<ncode<<"]"
			     << ".reg_assign(" << result << "["<<bit<<"]"
			     << ");" << endl;
	    }
      }

	/* Make the function call. */
      tgt_->defn << "      { bool flag;" << endl;
      tgt_->defn << "        flag = " << mangle(expr->name())<<"(thr);"<< endl;
      tgt_->defn << "        if (flag == false) return false;" << endl;
      tgt_->defn << "      }" << endl;

	/* rbits is the bits of the signal that hold the result. */
      string rbits = mangle(expr->result()->name()) + ".bits";

	/* Make a temporary to hold the result... */
      result = make_temp();
      tgt_->defn << "      vpip_bit_t " << result << "_bits["
		 << expr->expr_width() << "];" << endl;
      tgt_->defn << "      vvm_bitset_t " << result << "("
		 << result<<"_bits, " << expr->expr_width() << ");" << endl;

	/* Copy the result into the new temporary. */
      for (unsigned idx = 0 ;  idx < expr->expr_width() ;  idx += 1)
	    tgt_->defn << "      " << result << "_bits[" << idx << "] = "
		       << rbits << "[" << idx << "];" << endl;

}

void vvm_proc_rval::expr_unary(const NetEUnary*expr)
{
      expr->expr()->expr_scan(this);
      string tname = make_temp();

      tgt_->defn << "      vpip_bit_t " << tname << "_bits["
		 << expr->expr_width() << "];" << endl;
      tgt_->defn << "      vvm_bitset_t " << tname << "(" << tname<<"_bits, "
		 << expr->expr_width() << ");" << endl;

      switch (expr->op()) {
	  case '~':
	    tgt_->defn << "      vvm_unop_not(" << tname << "," << result <<
		  ");" << endl;
	    break;
	  case '&':
	    tgt_->defn << "      " << tname << "[0] "
		  "= vvm_unop_and("<<result<<");" << endl;
	    break;
	  case '|':
	    tgt_->defn << "      " << tname << "[0] "
		  "= vvm_unop_or("<<result<<");" << endl;
	    break;
	  case '^':
	    tgt_->defn << "      " << tname << "[0] "
		  "= vvm_unop_xor("<<result<<");" << endl;
	    break;
	  case '!':
	    tgt_->defn << "      " << tname << "[0] "
		  "= vvm_unop_lnot("<<result<<");" << endl;
	    break;
	  case '-':
	    tgt_->defn << "vvm_unop_uminus(" <<tname<< "," << result << ");" << endl;
	    break;
	  case 'N':
	    tgt_->defn << "      " << tname << "[0] "
		  "= vvm_unop_nor("<<result<<");" << endl;
	    break;
	  case 'X':
	    tgt_->defn << "      " << tname << "[0] "
		  "= vvm_unop_xnor("<<result<<");" << endl;
	    break;
	  default:
	    cerr << "vvm error: Unhandled unary op `" << expr->op() << "'"
		 << endl;
	    tgt_->defn << "#error \"" << expr->get_line() << ": vvm error: "
		  "Unhandled unary op: " << *expr << "\"" << endl;
	    tgt_->defn << result << ";" << endl;
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
      tgt_->defn << "      // " << expr->get_line() << ": expression node." << endl;
      tgt_->defn << "      vpip_bit_t " << result<<"_bits[" << expr->expr_width()
	  << "];" << endl;
      tgt_->defn << "      vvm_bitset_t " << result << "(" << result << "_bits, "
	  << expr->expr_width() << ");" << endl;

      switch (expr->op()) {
	  case 'a': // logical and (&&)
	    tgt_->defn << "      " << result << "[0] = vvm_binop_land("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'E': // ===
	    tgt_->defn << "      " << result << "[0] = vvm_binop_eeq("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'e': // ==
	    tgt_->defn << "      " << result << "[0] = vvm_binop_eq("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'G': // >=
	    if (expr->left()->has_sign() && expr->right()->has_sign()) {
		  tgt_->defn << "      " << result << "[0] = vvm_binop_ge_s("
			     << lres << "," << rres << ");" << endl;
	    } else {
		  tgt_->defn << "      " << result << "[0] = vvm_binop_ge("
			     << lres << "," << rres << ");" << endl;
	    }
	    break;
	  case 'l': // left shift(<<)
	    tgt_->defn << "      " << "vvm_binop_shiftl(" << result
		<< ", " << lres << "," << rres << ");" << endl;
	    break;
	  case 'L': // <=
	    if (expr->left()->has_sign() && expr->right()->has_sign()) {
		  tgt_->defn << "      " << result << "[0] = vvm_binop_le_s("
			     << lres << "," << rres << ");" << endl;
	    } else {
		  tgt_->defn << "      " << result << "[0] = vvm_binop_le("
			     << lres << "," << rres << ");" << endl;
	    }
	    break;
	  case 'N': // !==
	    tgt_->defn << "      " << result << "[0] = vvm_binop_nee("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'n':
	    tgt_->defn << "      " << result << "[0] = vvm_binop_ne("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case '<':
	    if (expr->left()->has_sign() && expr->right()->has_sign()) {
		  tgt_->defn << "      " << result << "[0] = vvm_binop_lt_s("
			     << lres << "," << rres << ");" << endl;
	    } else {
		  tgt_->defn << "      " << result << "[0] = vvm_binop_lt("
			     << lres << "," << rres << ");" << endl;
	    }
	    break;
	  case '>':
	    if (expr->left()->has_sign() && expr->right()->has_sign()) {
		  tgt_->defn << "      " << result << "[0] = vvm_binop_gt_s("
			     << lres << "," << rres << ");" << endl;
	    } else {
		  tgt_->defn << "      " << result << "[0] = vvm_binop_gt("
			     << lres << "," << rres << ");" << endl;
	    }
	    break;
	  case 'o': // logical or (||)
	    tgt_->defn << "      " << result << "[0] = vvm_binop_lor("
		<< lres << "," << rres << ");" << endl;
	    break;
	  case 'r': // right shift(>>)
	    tgt_->defn << "      " << "vvm_binop_shiftr(" << result
		<< ", " << lres << "," << rres << ");" << endl;
	    break;
	  case 'X':
	    tgt_->defn << "      " << "vvm_binop_xnor(" << result
		<< ", " << lres << "," << rres << ");" << endl;
	    break;
	  case '+':
	    tgt_->defn << "      " << "vvm_binop_plus(" << result
		<< ", " << lres << "," << rres << ");" << endl;
	    break;
	  case '-':
	    tgt_->defn << "      " << "vvm_binop_minus(" << result
		<< ", " << lres << "," << rres << ");" << endl;
	    break;
	  case '&':
	    tgt_->defn << "      " << "vvm_binop_and(" << result
		<< ", " << lres << ", " << rres << ");" << endl;
	    break;
	  case '|':
	    tgt_->defn << "      " << "vvm_binop_or(" << result
		<< ", " << lres << ", " << rres << ");" << endl;
	    break;
	  case '^':
	    tgt_->defn << "      " << "vvm_binop_xor(" << result
		<< ", " << lres << ", " << rres << ");" << endl;
	    break;
	  case '*':
	    tgt_->defn << "      " << "vvm_binop_mult(" << result
		<< "," << lres << "," << rres << ");" << endl;
	    break;
	  case '/':
	    tgt_->defn << "      " << "vvm_binop_idiv(" << result
		<< "," << lres << "," << rres << ");" << endl;
	    break;
	  case '%':
	    tgt_->defn << "      " << "vvm_binop_imod(" << result
		<< "," << lres << "," << rres << ");" << endl;
	    break;
	  default:
	    cerr << "vvm: Unhandled binary op `" << expr->op() << "': "
		 << *expr << endl;
	    tgt_->defn << "#error \"" << expr->get_line() << ": vvm error: "
		  "Unhandled binary op: " << *expr << "\"" << endl;
	    result = lres;
	    break;
      }
}

static string emit_proc_rval(target_vvm*tgt, const NetExpr*expr)
{
      vvm_proc_rval scan (tgt);
      expr->expr_scan(&scan);
      return scan.result;
}

/*
 * This is the default implementation for the expressions that are to
 * be passed as parameters to system tasks. This uses the proc_rval
 * class to make a vvm_bitset_t, then makes a vpiNumberConst out of that.
 */
void vvm_parm_rval::expr_default_(const NetExpr*expr)
{
      string rval = emit_proc_rval(tgt_, expr);

      string tmp = make_temp();
      tgt_->defn << "      struct __vpiNumberConst " << tmp << ";" << endl;
      tgt_->defn << "      vpip_make_number_const(&" << tmp << ", "
		 << rval << ".bits, " << expr->expr_width() << ");" << endl;
      result = "&" + tmp + ".base";
}

void vvm_parm_rval::expr_binary(const NetEBinary*expr)
{
      expr_default_(expr);
}

void vvm_parm_rval::expr_concat(const NetEConcat*expr)
{
      expr_default_(expr);
}

void vvm_parm_rval::expr_ufunc(const NetEUFunc*expr)
{
      expr_default_(expr);
}

void vvm_parm_rval::expr_unary(const NetEUnary*expr)
{
      expr_default_(expr);
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
			    << "const_bits_table+" << bit_idx << ", "
			    << width << ");" << endl;
      }

      ostrstream tmp;
      tmp << "&number_table[" << res << "].base" << ends;
      result = tmp.str();
      return;
}

void vvm_parm_rval::expr_sfunc(const NetESFunc*expr)
{
      if (!strcmp(expr->name(),"$time")) {
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
	    string rval = emit_proc_rval(tgt_, mem->index());
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

static string emit_parm_rval(target_vvm*tgt, const NetExpr*expr)
{
      vvm_parm_rval scan (tgt);
      expr->expr_scan(&scan);
      return scan.result;
}

bool target_vvm::start_design(const Design*mod)
{
      out.open(mod->get_flag("-o").c_str(), ios::out | ios::trunc);

      defn_name = tempnam(0, "ivldf");
      defn.open(defn_name, ios::in | ios::out | ios::trunc);

      init_code_name = tempnam(0, "ivlic");
      init_code.open(init_code_name, ios::in | ios::out | ios::trunc);

      start_code_name = tempnam(0, "ivlsc");
      start_code.open(start_code_name, ios::in | ios::out | ios::trunc);

      out << "# include \"vvm.h\"" << endl;
      out << "# include \"vvm_nexus.h\"" << endl;
      out << "# include \"vvm_gates.h\"" << endl;
      out << "# include \"vvm_signal.h\"" << endl;
      out << "# include \"vvm_func.h\"" << endl;
      out << "# include \"vvm_calltf.h\"" << endl;
      out << "# include \"vvm_thread.h\"" << endl;
      out << "# include \"vpi_user.h\"" << endl;
      out << "# include \"vpi_priv.h\"" << endl;

      signal_bit_counter = 0;
      signal_counter = 0;
      process_counter = 0;
      string_counter = 1;
      number_counter = 1;
      nexus_wire_counter = 1;
      selector_counter = 0;

      init_code << "static void design_init()" << endl;
      init_code << "{" << endl;
      init_code << "      vpip_init_simulation();"
		<< endl;
      init_code << "      vpip_time_scale("
		<< mod->get_precision() << ");" << endl;
      start_code << "static void design_start()" << endl;
      start_code << "{" << endl;

      return true;
}

void target_vvm::scope(const NetScope*scope)
{
      string hname = mangle(scope->name()) + "_scope";
      out << "// SCOPE: " << scope->name() << endl;
      out << "static struct __vpiScope " << hname << ";" << endl;

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
	    type_code << ", \"" << stresc(scope->name()) << "\");" << endl;

      if (const NetScope*par = scope->parent()) {
	    string pname = mangle(par->name()) + "_scope";
	    init_code << "      vpip_attach_to_scope(&" << pname << ", "
		      << "&" << hname << ".base);" << endl;
      }
}

void target_vvm::event(const NetEvent*event)
{
      string mname = mangle(event->full_name());
      out << "static vvm_sync " << mname << "; // "
	 << event->get_line() << ": event " << event->full_name() << endl;
}

int target_vvm::end_design(const Design*mod)
{
      if (string_counter > 0)
	    out << "static struct __vpiStringConst string_table[" <<
		  string_counter+1 << "];" << endl;
      if (number_counter > 0)
	    out << "static struct __vpiNumberConst number_table[" <<
		  number_counter+1 << "];" << endl;
      if (nexus_wire_counter > 0)
	    out << "static vvm_nexus nexus_wire_table[" <<
		  nexus_wire_counter << "];" << endl;
      if (signal_counter > 0)
	    out << "static vvm_signal_t signal_table[" <<
		  signal_counter << "];" << endl;
      if (signal_bit_counter > 0)
	    out << "static vpip_bit_t signal_bit_table[" <<
		  signal_bit_counter << "];" << endl;

      if (bits_table.count() > 0) {
	    out << "static vpip_bit_t const_bits_table[" << bits_table.count()
	       << "] = {";

	    for (unsigned idx = 0 ;  idx < bits_table.count() ;  idx += 1) {
		  if (idx%16 == 0) out << endl;
		  switch (bits_table.bit(idx)) {
		      case verinum::V0:
			out << " St0,";
			break;
		      case verinum::V1:
			out << " St1,";
			break;
		      case verinum::Vx:
			out << " StX,";
			break;
		      case verinum::Vz:
			out << " HiZ,";
			break;
		      default:
			out << "    ,";
			break;
		  }
	    }

	    out << endl << "};" << endl;
      }

      defn.close();

      out << "// **** Definition code" << endl;
      { ifstream rdefn (defn_name);
        out << rdefn.rdbuf();
      }
      unlink(defn_name);
      free(defn_name);
      defn_name = 0;
      out << "// **** end definition code" << endl;


      out << "// **** init_code" << endl;
      init_code << "}" << endl;
      init_code.close();
      { ifstream rinit_code (init_code_name);
        out << rinit_code.rdbuf();
      }
      unlink(init_code_name);
      free(init_code_name);
      init_code_name = 0;
      out << "// **** end init_code" << endl;


      out << "// **** start_code" << endl;
      start_code << "}" << endl;
      start_code.close();
      { ifstream rstart_code (start_code_name);
        out << rstart_code.rdbuf();
      }
      unlink(start_code_name);
      free(start_code_name);
      start_code_name = 0;
      out << "// **** end start_code" << endl;


      out << "main(int argc, char*argv[])" << endl << "{" << endl;

      out << "      vpip_set_vlog_info(argc, argv);" << endl;

      string vpi_module_path = mod->get_flag("VPI_MODULE_PATH");
      if (vpi_module_path.length() > 0)
	    out << "      vvm_set_module_path(\"" << vpi_module_path <<
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
	    out << "      vvm_load_vpi_module(\"" << stresc(name) << ".vpi\");" << endl;
      }
      out << "      design_init();" << endl;
      out << "      design_start();" << endl;
      out << "      vpip_simulation_run();" << endl;
      out << "}" << endl;
      out.close();

      return 0;
}

bool target_vvm::process(const NetProcTop*top)
{
      start_process(out, top);
      bool rc = top->statement()->emit_proc(this);
      end_process(out, top);
      return rc;
}

void target_vvm::signal(const NetNet*sig)
{
      string net_name = mangle(sig->name());

      unsigned*ncode_table = new unsigned [sig->pin_count()];
      const char*resolution_function = 0;

	/* By default, the nexus object uses a resolution
	   function that is suitable for simulating wire and tri
	   signals. If the signal is some other sort, the write
	   a resolution function into the nexus that properly
	   handles the different semantics. */

      switch (sig->type()) {
	  case NetNet::SUPPLY0:
	    resolution_function = "vvm_resolution_sup0";
	    break;
	  case NetNet::SUPPLY1:
	    resolution_function = "vvm_resolution_sup1";
	    break;
	  case NetNet::TRI0:
	    resolution_function = "vvm_resolution_tri0";
	    break;
	  case NetNet::TRI1:
	    resolution_function = "vvm_resolution_tri1";
	    break;
      }

	/* Scan the signals of the vector, getting an array of all the
	   nexus numbers. Do any nexus init if necessary. */

      for (unsigned idx = 0 ;  idx < sig->pin_count() ;  idx += 1) {
	    bool new_nexus_flag = false;
	    string nexus = sig->pin(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];
	    if (ncode == 0) {
		  nexus_wire_map[nexus] = (ncode = nexus_wire_counter);
		  nexus_wire_counter += 1;
		  new_nexus_flag = true;
	    }

	    ncode_table[idx] = ncode;


	      // Propogate the initial value to inputs throughout.
	    if (new_nexus_flag) {
		  verinum::V init = sig->pin(idx).nexus()->get_init();
		  emit_init_value_(sig->pin(idx), init);
	    }
      }

	/* Check to see if all the nexus numbers are increasing by one
	   for each bit of the signal. This is a common case and we
	   can generate optimal code for the situation. */
      bool increasing_flag = true;
      for (unsigned idx = 1 ;  idx < sig->pin_count() ;  idx += 1)
	    if (ncode_table[idx] != (ncode_table[idx-1] + 1))
		  increasing_flag = false;

      if (increasing_flag) {

	    unsigned base = ncode_table[0];
	    init_code << "      for (unsigned idx = 0 ;  idx < "
		      << sig->pin_count() << " ;  idx += 1) {" << endl;

	    init_code << "        nexus_wire_table[idx+"<<base
			    <<"].connect(&" << net_name << ", idx);"
			    << endl;

	    if (resolution_function)
		  init_code << "        nexus_wire_table[idx+" << base
			    << "].resolution_function = "
			    << resolution_function << ";" << endl;

	    init_code << "      }" << endl;

      } else {

	    for (unsigned idx = 0 ;  idx < sig->pin_count() ;  idx += 1) {
		  unsigned ncode = ncode_table[idx];

		  init_code << "      nexus_wire_table[" << ncode
			    <<"].connect(&" << net_name << ", " << idx << ");"
			    << endl;

		  if (resolution_function)
			init_code << "      nexus_wire_table[" << ncode
				  << "].resolution_function = "
				  << resolution_function << ";" << endl;

	    }
      }

      delete [] ncode_table;

      out << "#define " << net_name << " (signal_table[" <<
	    signal_counter << "])" << endl;

      signal_counter += 1;

      init_code << "      vpip_make_reg(&" << net_name
		<< ", \"" << stresc(sig->name()) << "\", signal_bit_table+"
		<< signal_bit_counter << ", " << sig->pin_count()
		<< ", " << (sig->get_signed()? "1" : "0") << ");" << endl;


      signal_bit_counter += sig->pin_count();

      if (! sig->local_flag()) {
	    const NetScope*scope = sig->scope();
	    assert(scope);
	    string sname = mangle(scope->name()) + "_scope";
	    init_code << "      vpip_attach_to_scope(&" << sname
		      << ", &" << net_name << ".base);" << endl;
      }


	/* Look at the initial values of the vector and see if they
	   can be assigned in a for loop. For this to work, all the
	   values must be the same. */
      verinum::V init = sig->pin(0).nexus()->get_init();
      bool uniform_flag = true;
      for (unsigned idx = 1 ;  idx < sig->pin_count() ;  idx += 1)
	    if (init != sig->pin(idx).nexus()->get_init())
		  uniform_flag = false;

      if (sig->pin_count() < 2)
	    uniform_flag = false;


      if (uniform_flag) {
	      /* Generate the short form. Assign all the initial
		 values of the vector using a for loop. */
	    init_code << "      for (unsigned idx = 0 ;  idx < "
		      << sig->pin_count() << " ;  idx += 1)" << endl;
	    init_code << "        " << mangle(sig->name())<<".init_P(idx, ";
	    switch (init) {
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

      } else {
	      /* Scan the signals of the vector, passing the initial
		 value to the inputs of all the connected devices. */
	    for (unsigned idx = 0 ;  idx < sig->pin_count() ;  idx += 1) {

		  init = sig->pin(idx).nexus()->get_init();
		  init_code << "      " << mangle(sig->name())
			    << ".init_P(" << idx << ", ";
		  switch (init) {
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
}

void target_vvm::memory(const NetMemory*mem)
{
      const string mname = mangle(mem->name());
      out << "static vvm_memory_t " << mname << ";"
	    " /* " << mem->name() << " */" << endl;
      init_code << "      vpip_make_memory(&" << mname << ", \"" <<
	    stresc(mem->name()) << "\", " << mem->width() << ", " <<
	    mem->count() << ");" << endl;
}

void target_vvm::task_def(const NetScope*scope)
{
      const NetTaskDef*def = scope->task_def();

      thread_step_ = 0;
      const string name = mangle(def->name());
      const string save_thread_class = thread_class_;
      thread_class_ = name;

      out << "static bool " << name << "_step_0_(vvm_thread*thr);" << endl;

      defn << "static bool " << name << "_step_0_(vvm_thread*thr) {" << endl;

      def->proc()->emit_proc(this);

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
void target_vvm::func_def(const NetScope*scope)
{
      const NetFuncDef*def = scope->func_def();
      thread_step_ = 0;
      const string name = mangle(def->name());

	// Flag that we are now in a function definition. Note that
	// function definitions cannot nest.
      assert(! function_def_flag_);
      function_def_flag_ = true;

      out << "// Function " << def->name() << endl;
      out << "static bool " << name << "(vvm_thread*thr);" << endl;

      defn << "// Function " << def->name() << endl;
      defn << "static bool " << name << "(vvm_thread*thr)" << endl;
      defn << "{" << endl;
      def->proc()->emit_proc(this);
      defn << "      return true;" << endl;
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

      const Nexus*nex = lnk.nexus();
      for (const Link*cur = nex->first_nlink()
		 ; cur ;  cur = cur->next_nlink()) {

	    if (cur == &lnk)
		  continue;

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

void target_vvm::lpm_add_sub(const NetAddSub*gate)
{
      out << "static vvm_add_sub " <<
	    mangle(gate->name()) << "(" << gate->width() << ");" << endl;

	/* Connect the DataA inputs. */

      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    if (! gate->pin_DataA(idx).is_linked())
		  continue;

	    string nexus = gate->pin_DataA(idx).nexus()->name();
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

	    string nexus = gate->pin_DataB(idx).nexus()->name();
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

	    string nexus = gate->pin_Result(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(" <<
		  mangle(gate->name()) << ".config_rout(" << idx <<
		  "));" << endl;
      }

	// Connect the carry output if necessary.
      if (gate->pin_Cout().is_linked()) {
	    string nexus = gate->pin_Cout().nexus()->name();
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

void target_vvm::lpm_clshift(const NetCLShift*gate)
{
      string mname = mangle(gate->name());

      out << "static vvm_clshift " << mname << "(" <<
	    gate->width() << "," << gate->width_dist() << ");" <<
	    endl;

	/* Connect the Data input pins... */
      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    string nexus = gate->pin_Data(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Data(" << idx << "));" << endl;
      }

	/* Connect the Distance input pins... */
      for (unsigned idx = 0 ;  idx < gate->width_dist() ;  idx += 1) {
	    string nexus = gate->pin_Distance(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Distance(" << idx << "));" << endl;
      }

	/* Connect the Direction pin... */
      if (gate->pin_Direction().is_linked()) {
	    string nexus = gate->pin_Direction().nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Direction(0));" << endl;
      }

	/* Connect the output drivers to the nexus nodes. */
      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    string nexus = gate->pin_Result(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }
}

void target_vvm::lpm_compare(const NetCompare*gate)
{
      string mname = mangle(gate->name());

      out << "static vvm_compare " << mname << "(" << gate->width() <<
	    ");" << endl;

	/* Connect DataA inputs... */
      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    string nexus = gate->pin_DataA(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataA(" << idx
		      << "));" << endl;
      }

	/* Connect DataB inputs... */
      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    string nexus = gate->pin_DataB(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataB(" << idx
		      << "));" << endl;
      }

      if (gate->pin_ALB().is_linked()) {
	    string nexus = gate->pin_ALB().nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_ALB_out());" << endl;
      }

      if (gate->pin_AGB().is_linked()) {
	    string nexus = gate->pin_AGB().nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_AGB_out());" << endl;
      }


      if (gate->pin_ALEB().is_linked()) {
	    string nexus = gate->pin_ALEB().nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_ALEB_out());" << endl;
      }

      if (gate->pin_AGEB().is_linked()) {
	    string nexus = gate->pin_AGEB().nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_AGEB_out());" << endl;
      }
}

void target_vvm::lpm_divide(const NetDivide*mul)
{
      string mname = mangle(mul->name());

      out << "static vvm_idiv " << mname << "(" << mul->width_r() <<
	    "," << mul->width_a() << "," << mul->width_b() << ");" << endl;


	/* Connect the DataA inputs... */
      for (unsigned idx = 0 ;  idx < mul->width_a() ;  idx += 1) {
	    string nexus = mul->pin_DataA(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataA("
		      << idx << "));" << endl;
      }

	/* Connect the DataB inputs... */
      for (unsigned idx = 0 ;  idx < mul->width_b() ;  idx += 1) {
	    string nexus = mul->pin_DataB(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataB("
		      << idx << "));" << endl;
      }

	/* Connect the output pins... */
      for (unsigned idx = 0 ;  idx < mul->width_r() ;  idx += 1) {
	    string nexus = mul->pin_Result(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }
}

void target_vvm::lpm_modulo(const NetModulo*mul)
{
      string mname = mangle(mul->name());

      out << "static vvm_imod " << mname << "(" << mul->width_r() <<
	    "," << mul->width_a() << "," << mul->width_b() << ");" << endl;


	/* Connect the DataA inputs... */
      for (unsigned idx = 0 ;  idx < mul->width_a() ;  idx += 1) {
	    string nexus = mul->pin_DataA(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataA("
		      << idx << "));" << endl;
      }

	/* Connect the DataB inputs... */
      for (unsigned idx = 0 ;  idx < mul->width_b() ;  idx += 1) {
	    string nexus = mul->pin_DataB(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataB("
		      << idx << "));" << endl;
      }

	/* Connect the output pins... */
      for (unsigned idx = 0 ;  idx < mul->width_r() ;  idx += 1) {
	    string nexus = mul->pin_Result(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }
}

void target_vvm::lpm_ff( const NetFF*gate)
{
      string nexus;
      unsigned ncode;
      string mname = mangle(gate->name());

      out << "static vvm_ff " << mname << "(" << gate->width() << ");" << endl;

	/* Connect the clock input... */

      nexus = gate->pin_Clock().nexus()->name();
      ncode = nexus_wire_map[nexus];

      init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		<< mname << ", " << mname << ".key_Clock());" << endl;

	/* Connect the Q output pins... */

      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    nexus = gate->pin_Q(idx).nexus()->name();
	    ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }


	/* Connect the Data input pins... */

      for (unsigned idx = 0 ;  idx < gate->width() ;  idx += 1) {
	    nexus = gate->pin_Data(idx).nexus()->name();
	    ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_Data(" << idx
		      << "));" << endl;
      }
}

void target_vvm::lpm_mult(const NetMult*mul)
{
      string mname = mangle(mul->name());

      out << "static vvm_mult " << mname << "(" << mul->width_r() <<
	    "," << mul->width_a() << "," << mul->width_b() << "," <<
	    mul->width_s() << ");" << endl;


	/* Connect the DataA inputs... */
      for (unsigned idx = 0 ;  idx < mul->width_a() ;  idx += 1) {
	    string nexus = mul->pin_DataA(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataA("
		      << idx << "));" << endl;
      }

	/* Connect the Datab inputs... */
      for (unsigned idx = 0 ;  idx < mul->width_b() ;  idx += 1) {
	    string nexus = mul->pin_DataB(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname << ".key_DataB("
		      << idx << "));" << endl;
      }

	/* Connect the output pins... */
      for (unsigned idx = 0 ;  idx < mul->width_r() ;  idx += 1) {
	    string nexus = mul->pin_Result(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }
}

void target_vvm::lpm_mux(const NetMux*mux)
{
      string mname = mangle(mux->name());

      out << "static vvm_mux " << mname << "(" << mux->width() << ","
	 << mux->size() << "," << mux->sel_width() << ");" << endl;

	/* Connect the select inputs... */
      for (unsigned idx = 0 ;  idx < mux->sel_width() ;  idx += 1) {
	    string nexus = mux->pin_Sel(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Sel(" << idx << "));" << endl;
      }

	/* Connect the data inputs... */
      for (unsigned idx = 0 ;  idx < mux->size() ;  idx += 1) {
	    for (unsigned wid = 0 ;  wid < mux->width() ;  wid += 1) {
		  string nexus = mux->pin_Data(wid, idx).nexus()->name();
		  unsigned ncode = nexus_wire_map[nexus];

		  init_code << "      nexus_wire_table["<<ncode<<"]"
			    << ".connect(&" << mname << ", "
			    << mname << ".key_Data("
			    << wid << "," << idx << "));" << endl;
	    }
      }

	/* Connect the outputs... */
      for (unsigned idx = 0 ;  idx < mux->width() ;  idx += 1) {
	    string nexus = mux->pin_Result(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }
}

void target_vvm::lpm_ram_dq(const NetRamDq*ram)
{
      string mname = mangle(ram->name());

      out << "static vvm_ram_dq<" << ram->width() << "," <<
	    ram->awidth() << "," << ram->size() << "> " << mname <<
	    "(&" << mangle(ram->mem()->name()) << ");" << endl;

      if (ram->pin_WE().is_linked()) {
	    string nexus = ram->pin_WE().nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_WE());" << endl;
      }

      if (ram->pin_InClock().is_linked()) {
	    string nexus = ram->pin_InClock().nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_InClock());" << endl;
      }

	/* Connect the address inputs... */
      for (unsigned idx = 0 ;  idx < ram->awidth() ;  idx += 1) {
	    if (! ram->pin_Address(idx).is_linked())
		  continue;

	    string nexus = ram->pin_Address(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Address(" << idx << "));" << endl;
      }

	/* Connect the data inputs... */
      for (unsigned idx = 0 ;  idx < ram->width() ;  idx += 1) {
	    if (! ram->pin_Data(idx).is_linked())
		  continue;

	    string nexus = ram->pin_Data(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << mname
		      << ".key_Data(" << idx << "));" << endl;
      }

	/* Connect the data outputs... */
      for (unsigned idx = 0 ;  idx < ram->width() ;  idx += 1) {
	    if (! ram->pin_Q(idx).is_linked())
		  continue;

	    string nexus = ram->pin_Q(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect("
		      << mname << ".config_rout(" << idx << "));" << endl;
      }

}

void target_vvm::logic(const NetLogic*gate)
{
      const string mname = mangle(gate->name());

	/* Draw the definition of the gate object. The exact type to
	   use depends on the gate type. Whatever the type, the basic
	   format is the same for all the boolean gates. */

      switch (gate->type()) {
	  case NetLogic::AND:
	    if ((gate->pin_count()-1) == 2)
		  out << "static vvm_and2 " << mname << "(";
	    else
		  out << "static vvm_and " << mname << "("
		      << (gate->pin_count()-1 ) << ", ";
	    break;
	  case NetLogic::BUF:
	    out << "static vvm_buf " << mname << "(";
	    break;
	  case NetLogic::BUFIF0:
	    out << "static vvm_bufif0 " << mname << "(";
	    break;
	  case NetLogic::BUFIF1:
	    out << "static vvm_bufif1 " << mname << "(";
	    break;
	  case NetLogic::NAND:
	    out << "static vvm_nand " << mname << "("
		<< (gate->pin_count()-1 ) << ", ";
	    break;
	  case NetLogic::NMOS:
	    out << "static vvm_nmos " << mname << "(";
	    break;
	  case NetLogic::NOR:
	    if ((gate->pin_count()-1) == 2)
		  out << "static vvm_nor2 " << mname << "(";
	    else
		  out << "static vvm_nor " << mname << "("
		      << (gate->pin_count()-1 ) << ", ";
	    break;
	  case NetLogic::NOT:
	    out << "static vvm_not " << mname << "(";
	    break;
	  case NetLogic::NOTIF0:
	    out << "static vvm_notif0 " << mname << "(";
	    break;
	  case NetLogic::NOTIF1:
	    out << "static vvm_notif1 " << mname << "(";
	    break;
	  case NetLogic::OR:
	    out << "static vvm_or " << mname << "("
		<< (gate->pin_count()-1 ) << ", ";
	    break;
	  case NetLogic::RNMOS:
	    out << "static vvm_rnmos " << mname << "(";
	    break;
	  case NetLogic::RPMOS:
	    out << "static vvm_rpmos " << mname << "(";
	    break;
	  case NetLogic::PMOS:
	    out << "static vvm_pmos " << mname << "(";
	    break;
	  case NetLogic::XNOR:
	    out << "static vvm_xnor " << mname << "("
		<< (gate->pin_count()-1 ) << ", ";
	    break;
	  case NetLogic::XOR:
	    out << "static vvm_xor " << mname << "("
		<< (gate->pin_count()-1 ) << ", ";
	    break;
	  default:
	    out << "#error \"internal ivl error:bad gate type for " <<
		  gate->name() << "\"" << endl;
      }

      out << gate->rise_time() << ");" << endl;

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

      { string nexus = gate->pin(0).nexus()->name();
        unsigned ncode = nexus_wire_map[nexus];
	init_code << "      nexus_wire_table[" << ncode <<
	      "].connect(&" << mangle(gate->name()) << ");" << endl;
      }

      for (unsigned idx = 1 ;  idx < gate->pin_count() ;  idx += 1) {
	    string nexus = gate->pin(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];
	    init_code << "      nexus_wire_table[" << ncode
		      << "].connect(&" << mangle(gate->name()) << ", "
		      << (idx-1) << ");" << endl;
      }
}

bool target_vvm::bufz(const NetBUFZ*gate)
{
      string mname = mangle(gate->name());
      string nexus;
      unsigned ncode;

      out << "static vvm_bufz " << mname
	  << "(" << gate->rise_time() << ");" << endl;
      nexus = gate->pin(0).nexus()->name();
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

      nexus = gate->pin(1).nexus()->name();
      ncode = nexus_wire_map[nexus];

      init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
	    mname << ",0);" << endl;

      return true;
}

void target_vvm::udp(const NetUDP*gate)
{
      string mname = mangle(gate->name());
      string nexus;
      unsigned ncode;

      // TODO: maintain a map<string,PUdp*> of tables already 
      // emited, and only print the table for each UDP once.

      out << "static const char*" << mname << "_ctab =" << endl;

      string inp;
      char outc;
      for (bool rc = gate->first(inp, outc)
		 ;  rc ;  rc = gate->next(inp,outc)) {

	    out << "    \"" << inp << outc << "\"" << endl;
      }
      out << "    ;" << endl;

      out << (gate->is_sequential() ? "static vvm_udp_sequ1 " 
	                            : "static vvm_udp_comb " )
         << mname << "("
	 << (gate->pin_count()-1) << ", " << mname<<"_ctab);" << endl;

	/* Connect the output of the device... */
      nexus = gate->pin(0).nexus()->name();
      ncode = nexus_wire_map[nexus];
      init_code << "      nexus_wire_table["<<ncode<<"].connect(&" <<
	    mname << ");" << endl;

	/* Connect the inputs of the device... */
      for (unsigned idx = 1 ;  idx < gate->pin_count() ;  idx += 1) {
	    if (! gate->pin(idx).is_linked())
		  continue;

	    nexus = gate->pin(idx).nexus()->name();
	    ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << "," << (idx-1) << ");" << endl;
      }

}


void target_vvm::net_case_cmp(const NetCaseCmp*gate)
{
      string mname = mangle(gate->name());
      string nexus;
      unsigned ncode;

      assert(gate->pin_count() == 3);
      out << "static vvm_eeq " << mname << "(" <<
	    gate->rise_time() << ");" << endl;

	/* Connect the output pin */
      nexus = gate->pin(0).nexus()->name();
      ncode = nexus_wire_map[nexus];
      init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		<< mname << ");" << endl;

	/* Connect the first input */
      nexus = gate->pin(1).nexus()->name();
      ncode = nexus_wire_map[nexus];
      init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		<< mname << ", 0);" << endl;

	/* Connect the second input */
      nexus = gate->pin(2).nexus()->name();
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
bool target_vvm::net_cassign(const NetCAssign*dev)
{
      string mname = mangle(dev->name());

      out << "static vvm_force " << mname << "(" << dev->pin_count()
	 << ");" << endl;

      for (unsigned idx = 0 ;  idx < dev->pin_count() ;  idx += 1) {
	    string nexus = dev->pin(idx).nexus()->name();
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
bool target_vvm::net_const(const NetConst*gate)
{
      const string mname = mangle(gate->name());

      out << "static vvm_nexus::drive_t " << mname
	 << "[" << gate->pin_count() << "];" << endl;

      for (unsigned idx = 0 ;  idx < gate->pin_count() ;  idx += 1) {
	    string nexus = gate->pin(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    const char*val_str = vvm_val_name(gate->value(idx),
					      gate->pin(idx).drive0(),
					      gate->pin(idx).drive1());

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << "["<<idx<<"]);" << endl;
	    start_code << "      " << mname << "["<<idx<<"].set_value("
		       << val_str << ");" << endl;
      }

      return true;
}


bool target_vvm::net_force(const NetForce*dev)
{
      string mname = mangle(dev->name());

      out << "static vvm_force " << mname << "(" << dev->pin_count()
	 << ");" << endl;

      for (unsigned idx = 0 ;  idx < dev->pin_count() ;  idx += 1) {
	    string nexus = dev->pin(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    init_code << "      nexus_wire_table["<<ncode<<"].connect(&"
		      << mname << ", " << idx << ");" << endl;
      }

      return true;
}

void target_vvm::net_probe(const NetEvProbe*net)
{
      string mname = mangle(net->name());
      string mevent = mangle(net->event()->full_name());

      switch (net->edge()) {
	  case NetEvProbe::POSEDGE:
	    assert(net->pin_count() == 1);
	    out << "static vvm_posedge " << mname
	       << "(&" << mevent << ");" << endl;
	    break;

	  case NetEvProbe::NEGEDGE:
	    assert(net->pin_count() == 1);
	    out << "static vvm_negedge " << mname
	       << "(&" << mevent << ");" << endl;
	    break;


	  case NetEvProbe::ANYEDGE:
	    out << "static vvm_anyedge " << mname
	       << "(&" << mevent << ", " << net->pin_count() << ");" << endl;
	    break;
      }


	/* Connect this device as a receiver to the nexus that is my
	   source. Write the connect calls into the init code. */

      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    string nexus = net->pin(idx).nexus()->name();
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
      init_code << "      " << thread_class_ << ".scope = &"
		<< mangle(proc->scope()->name()) << "_scope;" << endl;
      init_code << "      " << thread_class_ << ".thread_yield();" << endl;


      defn << "static bool " << thread_class_ << "_step_0_(vvm_thread*thr)"
	   << endl << "{" << endl;
}

/*
 * This method handles the special case of the assignment of a
 * constant r-value to the l-value. In this case, I can set specific
 * values to each of the bits instead of calculating bit values or
 * even reading values from a vpip_bit_t elsewhere.
 */
void target_vvm::proc_assign_rval(const NetAssign_*lv,
				  const NetEConst*rv,
				  unsigned off)
{
      const verinum value = rv->value();

	/* This condition catches the special case of assigning to a
	   non-constant bit select. This cal be something like:

		a[idx] = x;

	   For this sort of assignment, I only need a single bit of
	   the r-value. That bit is written into a single bit of the
	   target using a generated switch statement, where each case
	   of the switch assignes to a specific nexus. This is not
	   unreasonable because there aren't typically all that many
	   bits in the l-value. */

      if (lv->bmux()) {

	      // This is a bit select. Assign the low bit of the
	      // constant to the selected bit of the lval.

	    const char*rval = vvm_val_name(value.get(off),
					   Link::STRONG,
					   Link::STRONG);

	    string bval = emit_proc_rval(this, lv->bmux());

	    defn << "      switch (" << bval
		 << ".as_unsigned()) {" << endl;

	    for (unsigned idx = 0; idx < lv->pin_count(); idx += 1) {

		  string nexus = lv->pin(idx).nexus()->name();
		  unsigned ncode = nexus_wire_map[nexus];

		  defn << "      case " << idx << ":" << endl;

		  defn << "        nexus_wire_table["<<ncode<<"]"
		       << ".reg_assign(" << rval << ");" << endl;
		  defn << "        break;" << endl;

	    }

	    defn << "      }" << endl;
	    return;
      }


	/* We've handled the case of bit selects, so here we know that
	   we are doing a good ol' assignment to an l-value. So for
	   the entire width of the l-value, assign constant bit values
	   to the appropriate nexus.

	   First make a map of the nexa that are going to receive the
	   constant value. In the process, check to se if the value is
	   uniform and the nexa are sequential.

	   If the nexa are sequential and uniform, write a for loop
	   that does the assignment. This is an optimization that
	   reduces the size of the generated C++. */

      unsigned*nexus_map = new unsigned[lv->pin_count()];

      bool sequential_flag = true;
      bool uniform_flag = true;
      verinum::V val = off < value.len() 
		  ? value.get(off)
		  : verinum::V0;
      unsigned zeros_start = 0;

      for (unsigned idx = 0 ;  idx < lv->pin_count() ;  idx += 1) {
	    string nexus = lv->pin(idx).nexus()->name();
	    nexus_map[idx] = nexus_wire_map[nexus];

	    verinum::V tmp = (idx+off) < value.len() 
		  ? value.get(idx+off)
		  : verinum::V0;

	    if (tmp != verinum::V0)
		  zeros_start = idx + 1;

	    if (idx > 0) {
		  if (nexus_map[idx] != (nexus_map[idx-1] + 1))
			sequential_flag = false;

		  if (tmp != val)
			uniform_flag = false;
	    }
      }


      if (sequential_flag && uniform_flag && (lv->pin_count() > 1)) {

	    const char*rval = vvm_val_name(val, Link::STRONG, Link::STRONG);
	    unsigned base = nexus_map[0];

	    defn << "      for (unsigned idx = 0 ;  idx < "
		 << lv->pin_count() << " ;  idx += 1)" << endl;

	    defn << "        nexus_wire_table[idx+" <<base<< "]"
		 << ".reg_assign(" << rval << ");" << endl;

      } else if (sequential_flag && (zeros_start < lv->pin_count())) {

	      /* If the nexa are sequential and the high bits are all
		 zeros, then we can write simple reg_assign statements
		 to take care of the low bits, then write a for loop
		 to fill in all the high zero bits.

		 This is interesting as it is common to assign small
		 integers to wide vectors. */

	    const char*rval;

	    for (unsigned idx = 0 ;  idx < zeros_start ;  idx += 1) {
		  unsigned ncode = nexus_map[idx];

		  val = (idx+off) < value.len() 
			? value.get(idx+off)
			: verinum::V0;
		  rval = vvm_val_name(val, Link::STRONG, Link::STRONG);

		  defn << "      nexus_wire_table[" <<ncode<< "]"
		       << ".reg_assign(" << rval << ");" << endl;
	    }

	    rval = vvm_val_name(verinum::V0, Link::STRONG, Link::STRONG);

	    unsigned base = nexus_map[zeros_start];

	    defn << "      for (unsigned idx = 0 ;  idx < "
		 << (lv->pin_count()-zeros_start) << " ;  idx += 1)" << endl;

	    defn << "        nexus_wire_table[idx+" <<base<< "]"
		 << ".reg_assign(" << rval << ");" << endl;


      } else for (unsigned idx = 0 ;  idx < lv->pin_count() ;  idx += 1) {
	    unsigned ncode = nexus_map[idx];

	    val = (idx+off) < value.len() 
		  ? value.get(idx+off)
		  : verinum::V0;
	    const char*rval = vvm_val_name(val, Link::STRONG, Link::STRONG);

	    defn << "      nexus_wire_table[" <<ncode<< "]"
		 << ".reg_assign(" << rval << ");" << endl;
      }

      delete[]nexus_map;
}

/*
 * This method does the grunt work of generating an assignment given a
 * generated rval result.
 */
void target_vvm::proc_assign_rval(const NetAssign_*lv,
				  const string&rval,
				  unsigned wid, unsigned off)
{
      assert(lv);

	/* Now, if there is a mux on the l-value, generate a code to
	   assign a single bit to one of the bits of the
	   l-value. Otherwise, generate code for a complete
	   assignment. */

      if (lv->bmux()) {

	      // This is a bit select. Assign the low bit of the rval
	      // to the selected bit of the lval.
	    string bval = emit_proc_rval(this, lv->bmux());

	    defn << "      switch (" << bval << ".as_unsigned()) {" << endl;

	    for (unsigned idx = 0 ;  idx < lv->pin_count() ;  idx += 1) {

		  string nexus = lv->pin(idx).nexus()->name();
		  unsigned ncode = nexus_wire_map[nexus];

		  defn << "      case " << idx << ":" << endl;

		  defn << "        nexus_wire_table["<<ncode<<"]"
		       << ".reg_assign(" << rval << "["<<off<<"]);" << endl;
		  defn << "        break;" << endl;

	    }

	    defn << "      }" << endl;

      } else {
	    unsigned min_count = lv->pin_count();
	    if ((wid-off) < min_count)
		  min_count = wid - off;

	      /* First, make a map of the nexa that are connected to
		 the l-value. */

	    bool sequential_flag = true;
	    unsigned*nexus_map = new unsigned[lv->pin_count()];
	    for (unsigned idx = 0 ;  idx < lv->pin_count() ;  idx += 1) {
		  string nexus = lv->pin(idx).nexus()->name();
		  nexus_map[idx] = nexus_wire_map[nexus];

		  if ((idx >= 1) && (nexus_map[idx] != (nexus_map[idx-1]+1)))
			sequential_flag = false;
	    }

	    if (sequential_flag && (min_count > 1)) {
		  unsigned base = nexus_map[0];

		  defn << "      for (unsigned idx = 0 ;  idx < "
		       << min_count << "; idx += 1)" << endl;

		  defn << "        nexus_wire_table[idx+"<<base<<"]"
		       << ".reg_assign(" << rval << "[idx]);" << endl;

	    } else {
		  for (unsigned idx = 0 ;  idx < min_count ;  idx += 1) {
			unsigned ncode = nexus_map[idx];
			defn << "      nexus_wire_table["<<ncode
			     <<"].reg_assign(" << rval << "["
			     << (idx+off) << "]);" << endl;
		  }
	    }

	    for (unsigned idx = min_count; idx < lv->pin_count(); idx += 1) {
		  unsigned ncode = nexus_map[idx];
		  defn << "      nexus_wire_table["<<ncode<<"]"
		       << ".reg_assign(St0);" << endl;
	    }

	    delete[] nexus_map;
      }
}

/*
 * This method generates code for a procedural assignment. The lval is
 * a signal, but the assignment should generate code to go to all the
 * connected devices/events.
 */
void target_vvm::proc_assign(const NetAssign*net)
{

	/* Detect the very special (and very common) case that the
	   rvalue is a constant value. In this case, there is no
	   reason to go scan the expression, and in the process
	   generate bunches of temporaries. */

      if (const NetEConst*rc = dynamic_cast<const NetEConst*>(net->rval())) {

	    const NetAssign_*cur = net->l_val(0);
	    unsigned off = 0;
	    unsigned idx = 0;
	    while (cur != 0) {
		  proc_assign_rval(cur, rc, off);
		  off += cur->lwidth();
		  idx += 1;
		  cur = net->l_val(idx);
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

	    if (net->lwidth() > rs->pin_count()) {
		  rval = emit_proc_rval(this, net->rval());

	    } else {

		  rval = mangle(rs->name()) + ".bits";
	    }

      } else {

	    rval = emit_proc_rval(this, net->rval());
      }


      defn << "      // " << net->get_line() << ": " << endl;

      { const NetAssign_*cur = net->l_val(0);
        unsigned wid = net->rval()->expr_width();
        unsigned off = 0;
	unsigned idx = 0;
	while (cur != 0) {
	      proc_assign_rval(cur, rval, wid, off);
	      off += cur->lwidth();
	      idx += 1;
	      cur = net->l_val(idx);
	}
      }
}

/*
 * Generate an assignment to a memory location. This causes the index
 * expression to be evaluated (run-time) and the index used as the
 * location to store the value.
 */
void target_vvm::proc_assign_mem(const NetAssignMem*amem)
{
	/* make a temporary to reference the index signal. */
      string index = emit_proc_rval(this, amem->index());

	/* Evaluate the rval that gets written into the memory word. */
      string rval = emit_proc_rval(this, amem->rval());


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


/*
 * This method handles the special case of the assignment of a
 * constant r-value to the l-value. In this case, I can set specific
 * values to each of the bits instead of calculating bit values or
 * even reading values from a vpip_bit_t elsewhere.
 */
void target_vvm::proc_assign_nb_rval(const NetAssign_*lv,
				     const NetEConst*rv,
				     unsigned off)
{
      const verinum value = rv->value();
      const unsigned rise_time = lv->rise_time();

	/* This condition catches the special case of assigning to a
	   non-constant bit select. This cal be something like:

		a[idx] = x;

	   For this sort of assignment, I only need a single bit of
	   the r-value. That bit is written into a single bit of the
	   target using a generated switch statement, where each case
	   of the switch assignes to a specific nexus. This is not
	   unreasonable because there aren't typically all that many
	   bits in the l-value. */

      if (lv->bmux()) {

	      // This is a bit select. Assign the low bit of the
	      // constant to the selected bit of the lval.

	    const char*rval = vvm_val_name(value.get(off),
					   Link::STRONG,
					   Link::STRONG);

	    string bval = emit_proc_rval(this, lv->bmux());

	    defn << "      switch (" << bval
		 << ".as_unsigned()) {" << endl;

	    for (unsigned idx = 0; idx < lv->pin_count(); idx += 1) {

		  string nexus = lv->pin(idx).nexus()->name();
		  unsigned ncode = nexus_wire_map[nexus];

		  defn << "      case " << idx << ":" << endl;

		  defn << "        vvm_delayed_assign(nexus_wire_table["
		       <<ncode<<"], " << rval << ", " <<rise_time<< ");"
		       << endl;

		  defn << "        break;" << endl;

	    }

	    defn << "      }" << endl;
	    return;
      }


	/* We've handled the case of bit selects, so here we know that
	   we are doing a good ol' assignment to an l-value. So for
	   the entire width of the l-value, assign constant bit values
	   to the appropriate nexus. */

      for (unsigned idx = 0 ;  idx < lv->pin_count() ;  idx += 1) {
	    string nexus = lv->pin(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    verinum::V val = (idx+off) < value.len() 
		  ? value.get(idx+off)
		  : verinum::V0;
	    const char*rval = vvm_val_name(val, Link::STRONG, Link::STRONG);

	    defn << "        vvm_delayed_assign(nexus_wire_table["
		 <<ncode<< "], " << rval << ", " <<rise_time<< ");" << endl;
      }
}


/*
 * This method does the grunt work of generating an assignment given a
 * generated rval result.
 */
void target_vvm::proc_assign_nb_rval(const NetAssign_*lv,
				     const string&rval,
				     unsigned wid, unsigned off)
{
      assert(lv);
      const unsigned rise_time = lv->rise_time();

	/* Now, if there is a mux on the l-value, generate a code to
	   assign a single bit to one of the bits of the
	   l-value. Otherwise, generate code for a complete
	   assignment. */

      if (lv->bmux()) {

	      // This is a bit select. Assign the low bit of the rval
	      // to the selected bit of the lval.
	    string bval = emit_proc_rval(this, lv->bmux());

	    defn << "      switch (" << bval << ".as_unsigned()) {" << endl;

	    for (unsigned idx = 0 ;  idx < lv->pin_count() ;  idx += 1) {

		  string nexus = lv->pin(idx).nexus()->name();
		  unsigned ncode = nexus_wire_map[nexus];

		  defn << "      case " << idx << ":" << endl;

		  defn << "        vvm_delayed_assign(nexus_wire_table["
		       <<ncode<<"], " << rval << "["<<off<<"], "
		       <<rise_time<< ");" << endl;
		  defn << "        break;" << endl;

	    }

	    defn << "      }" << endl;

      } else {
	    unsigned min_count = lv->pin_count();
	    if ((wid-off) < min_count)
		  min_count = wid - off;

	    for (unsigned idx = 0 ;  idx < min_count ;  idx += 1) {
		  string nexus = lv->pin(idx).nexus()->name();
		  unsigned ncode = nexus_wire_map[nexus];
		  defn << "      vvm_delayed_assign(nexus_wire_table["
		       <<ncode<<"], "
		       << rval << "[" << (idx+off) << "], "
		       <<rise_time<< ");" << endl;
	    }

	    for (unsigned idx = min_count; idx < lv->pin_count(); idx += 1) {
		  string nexus = lv->pin(idx).nexus()->name();
		  unsigned ncode = nexus_wire_map[nexus];
		  defn << "      vvm_delayed_assign(nexus_wire_table["
		       <<ncode<<"], St0, " <<rise_time<< ");" << endl;
	    }
      }
}

void target_vvm::proc_assign_nb(const NetAssignNB*net)
{

	/* Detect the very special (and very common) case that the
	   rvalue is a constant value. In this case, there is no
	   reason to go scan the expression, and in the process
	   generate bunches of temporaries. */

      if (const NetEConst*rc = dynamic_cast<const NetEConst*>(net->rval())) {

	    const NetAssign_*cur = net->l_val(0);
	    unsigned off = 0;
	    unsigned idx = 0;
	    while (cur != 0) {
		  proc_assign_nb_rval(cur, rc, off);
		  off += cur->lwidth();
		  idx += 1;
		  cur = net->l_val(idx);
	    }

	    return;
      }


      string rval;
      if (net->lwidth() > net->rval()->expr_width()) {
	    cerr << net->get_line() << ": internal error: "
		 << "lvalue width is " << net->lwidth() << ", "
		 << "rvalue width is " << net->rval()->expr_width()
		 << "." << endl;
      }
      assert(net->lwidth() <= net->rval()->expr_width());


	/* Handle another special case, that of an r-value that is a
	   simple identifier. In this case we don't need to generate
	   the vvm_bitset_t but can pull the result directly out of
	   the identifier memory. It is OK to turn the r-value string
	   into a simple vpip_bit_t array (the .bits member of the
	   signal) because we know that we will only be using the []
	   operator on it. */

      if (const NetESignal*rs = dynamic_cast<const NetESignal*>(net->rval())) {

	    if (net->lwidth() > rs->pin_count()) {
		  rval = emit_proc_rval(this, net->rval());

	    } else {

		  rval = mangle(rs->name()) + ".bits";
	    }

      } else {

	    rval = emit_proc_rval(this, net->rval());
      }


      defn << "      // " << net->get_line() << ": " << endl;

      { const NetAssign_*cur = net->l_val(0);
        unsigned wid = net->rval()->expr_width();
        unsigned off = 0;
	unsigned idx = 0;
	while (cur != 0) {
	      proc_assign_nb_rval(cur, rval, wid, off);
	      off += cur->lwidth();
	      idx += 1;
	      cur = net->l_val(idx);
	}
      }
}

void target_vvm::proc_assign_mem_nb(const NetAssignMemNB*amem)
{
	/* make a temporary to reference the index signal. */
      string index = emit_proc_rval(this, amem->index());


	/* Evaluate the rval that gets written into the memory word. */
      string rval = emit_proc_rval(this, amem->rval());

      const NetMemory*mem = amem->memory();

      defn << "      /* " << amem->get_line() << " */" << endl;

	/* Note here that the vvm_memory_t::assign_nb constructor will
	   pad the rval with St0 if it is not as wide as the memory
	   word. If this is not what is desired, then it should have
	   been fixed up by semantic analysis anyhow. */

      defn << "      (new vvm_memory_t::assign_nb(" << mangle(mem->name())
	   << ", " << index << ".as_unsigned(), " << rval <<
	    ")) -> schedule();" << endl;
}

bool target_vvm::proc_block(const NetBlock*net)
{
      if (net->type() == NetBlock::SEQU) {
	    net->emit_recurse(this);
	    return true;
      }

      unsigned exit_step = thread_step_ + 1;

      thread_step_ += 1;

      const NetProc*cur;
      unsigned cnt = 0;
      unsigned idx;

	// Declare the exit step...

      out << "static bool " << thread_class_ << "_step_" << exit_step
	 << "_(vvm_thread*thr);" << endl;


	// Declare the first steps for all the threads to be created,
	// and count those threads while I'm at it.

      for (cur = net->proc_first() ;  cur ;  cur = net->proc_next(cur)) {
	    cnt += 1;
	    out << "static bool " << thread_class_ << "_step_"
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
	    defn << "      thr->callee_["<<idx<<"].scope = "
		 << "thr->scope;" << endl;
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

	    cur->emit_proc(this);

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
void target_vvm::proc_case(const NetCase*net)
{
      if (function_def_flag_) {
	    proc_case_fun(out, net);
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
      string expr = emit_proc_rval(this, net->expr());

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
	    string guard = emit_proc_rval(this, net->expr(idx));

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

	    out << "static bool " << thread_class_ << "_step_"
		 << step_num << "_(vvm_thread*thr);" << endl;

	    defn << "static bool " << thread_class_ << "_step_"
		 << step_num << "_(vvm_thread*thr) {" << endl;
	    if (net->stat(idx))
		  net->stat(idx)->emit_proc(this);
	    defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
		  exit_step << "_;" << endl;
	    defn << "      return true;" << endl;
	    defn << "}" << endl;
      }

	/* If there is a default case, generate the default step. */
      if (default_idx < net->nitems()) {
	    step_num += 1;

	    out << "static bool " << thread_class_ << "_step_"
	       << step_num << "_(vvm_thread*thr);" << endl;

	    defn << "static bool " << thread_class_ << "_step_"
		 << step_num << "_(vvm_thread*thr) {" << endl;
	    if (net->stat(default_idx))
		  net->stat(default_idx)->emit_proc(this);
	    defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
		  exit_step << "_;" << endl;
	    defn << "      return true;" << endl;
	    defn << "}" << endl;
      }

	/* Finally, start the exit step. */

      out << "static bool " << thread_class_ << "_step_"
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

      string expr = emit_proc_rval(this, net->expr());

      unsigned default_idx = net->nitems();
      for (unsigned idx = 0 ;  idx < net->nitems() ;  idx += 1) {

	      // don't emit the default case here. Save it for the
	      // last else clause.
	    if (net->expr(idx) == 0) {
		  default_idx = idx;
		  continue;
	    }

	    string guard = emit_proc_rval(this, net->expr(idx));

	    defn << "      if (B_IS1(" << test_func << "(" <<
		  guard << "," << expr << "))) {" << endl;
	    if (net->stat(idx))
		  net->stat(idx)->emit_proc(this);
	    defn << "      break; }" << endl;
      }

      if ((default_idx < net->nitems()) && net->stat(default_idx)) {
	    net->stat(default_idx)->emit_proc(this);
      }

      defn << "      /* " << net->get_line() << ": end case (" <<
	    *net->expr() << ") */" << endl;
      defn << "      } while(0);" << endl;
}

bool target_vvm::proc_cassign(const NetCAssign*dev)
{
      const string mname = mangle(dev->name());

      for (unsigned idx = 0 ;  idx < dev->pin_count() ;  idx += 1) {
	    string nexus = dev->lval_pin(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    defn << "      " << mname << ".assign("<<idx<<", "
		 << "nexus_wire_table+"<<ncode << ");" << endl;
      }

      return true;
}

void target_vvm::proc_condit(const NetCondit*net)
{
      if (function_def_flag_) {
	    proc_condit_fun(out, net);
	    return;
      }

      string expr = emit_proc_rval(this, net->expr());

      unsigned if_step   = ++thread_step_;
      unsigned else_step = ++thread_step_;
      unsigned out_step  = ++thread_step_;

	/* Declare new steps that I am going to create. */

      out << "static bool " << thread_class_ << "_step_"
	 << if_step << "_(vvm_thread*thr);" << endl;

      out << "static bool " << thread_class_ << "_step_"
	 << else_step << "_(vvm_thread*thr);" << endl;

      out << "static bool " << thread_class_ << "_step_"
	 << out_step << "_(vvm_thread*thr);" << endl;

      if (net->expr()->expr_width() == 1)
	    defn << "      if (B_IS1(" << expr << "[0]))" << endl;
      else
	    defn << "      if (B_IS1(vvm_unop_or(" << expr << ")))" << endl;

      defn << "        thr->step_ = &" << thread_class_ << "_step_" <<
	    if_step << "_;" << endl;
      defn << "      else" << endl;
      defn << "        thr->step_ = &" << thread_class_ << "_step_" <<
	    else_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      defn << "static bool " << thread_class_ << "_step_" << if_step <<
	    "_(vvm_thread*thr) {" << endl;
      net->emit_recurse_if(this);
      defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
	    out_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      defn << "static bool " << thread_class_ << "_step_" << else_step <<
	    "_(vvm_thread*thr) {" << endl;
      net->emit_recurse_else(this);
      defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
	    out_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      defn << "static bool " << thread_class_ << "_step_" << out_step <<
	    "_(vvm_thread*thr) {" << endl;
}

void target_vvm::proc_condit_fun(ostream&os, const NetCondit*net)
{
      string expr = emit_proc_rval(this, net->expr());

      defn << "      // " << net->get_line() << ": conditional (if-else)"
	   << endl;
      defn << "      if (B_IS1(" << expr << "[0])) {" << endl;
      net->emit_recurse_if(this);
      defn << "      } else {" << endl;
      net->emit_recurse_else(this);
      defn << "      }" << endl;
}

bool target_vvm::proc_deassign(const NetDeassign*dev)
{
      const NetNet*lval = dev->lval();
      for (unsigned idx = 0 ;  idx < lval->pin_count() ;  idx += 1) {
	    string nexus = lval->pin(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    defn << "      nexus_wire_table["<<ncode<<"].deassign();"
		 << endl;
      }

      return true;
}

bool target_vvm::proc_force(const NetForce*dev)
{
      const string mname = mangle(dev->name());

      for (unsigned idx = 0 ;  idx < dev->pin_count() ;  idx += 1) {
	    string nexus = dev->lval_pin(idx).nexus()->name();
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
void target_vvm::proc_forever(const NetForever*net)
{
      unsigned top_step = ++thread_step_;
      unsigned out_step = ++thread_step_;

      defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
	    top_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      out << "static bool " << thread_class_ << "_step_"
	 << top_step << "_(vvm_thread*thr);" << endl;

      defn << "static bool " << thread_class_ << "_step_"
	   << top_step << "_(vvm_thread*thr) {" << endl;

      net->emit_recurse(this);
      defn << "      thr->step_ = &" << thread_class_ << "_step_" <<
	    top_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

	/* Generate a loop out step to catch unreachable stuff afer
	   the loop. */

      out << "static bool " << thread_class_ << "_step_"
	 << out_step << "_(vvm_thread*thr);" << endl;
     
      defn << "static bool " << thread_class_ << "_step_"
	   << out_step << "_(vvm_thread*thr) {" << endl;
}

bool target_vvm::proc_release(const NetRelease*dev)
{
      const NetNet*lval = dev->lval();
      for (unsigned idx = 0 ;  idx < lval->pin_count() ;  idx += 1) {
	    string nexus = lval->pin(idx).nexus()->name();
	    unsigned ncode = nexus_wire_map[nexus];

	    defn << "      nexus_wire_table["<<ncode<<"].release();"
		 << endl;
      }

      return true;
}

void target_vvm::proc_repeat( const NetRepeat*net)
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
	    expr = emit_proc_rval(this, net->expr());
	    expr = expr + ".as_unsigned()";
      }

	/* Declare a variable to use as a loop index. */
      out << "static unsigned " << thread_class_ << "_step_"
	 << top_step << "_idx_;" << endl;

	/* Declare the top step. */
      out << "static bool " << thread_class_ << "_step_"
	 << top_step << "_(vvm_thread*thr);" << endl;

	/* Declare the exit step. */
      out << "static bool " << thread_class_ << "_step_"
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

      net->emit_recurse(this);

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
void target_vvm::proc_stask(const NetSTask*net)
{
	/* Handle the special case of a system task without any
	   parameters. we don't need a parameter array for this. */

      if (net->nparms() == 0) {
	    defn << "      vpip_calltask(thr->scope, \"" << stresc(net->name())
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
		  val = emit_parm_rval(this, net->parm(idx));

	    } else {
		  val = string("&(vpip_get_null()->base)");
	    }

	    defn << "      " << ptmp << "[" << idx << "] = " << val << ";"
	       << endl;
      }

      defn << "      vpip_calltask(thr->scope, \"" << stresc(net->name()) << "\", "
	   << net->nparms() << ", " << ptmp << ");" << endl;
      defn << "      if (vpip_finished()) return false;" << endl;
}

bool target_vvm::proc_trigger(const NetEvTrig*trig)
{
      const NetEvent*ev = trig->event();
      assert(ev);

      string ename = mangle(ev->full_name());

      defn << "      " << ename << ".wakeup(); // "
	   << trig->get_line() << ": -> " << ev->full_name() << endl;

      return true;
}

void target_vvm::proc_utask(const NetUTask*net)
{
      unsigned out_step = ++thread_step_;
      const string name = mangle(net->name());

      out << "static bool " << thread_class_ << "_step_"
	 << out_step << "_(vvm_thread*thr);" << endl;

      defn << "      assert(thr->callee_ == 0);" << endl;
      defn << "      thr->callee_ = new vvm_thread;" << endl;
      defn << "      thr->callee_->back_ = thr;" << endl;
      defn << "      thr->callee_->step_ = &" << name << "_step_0_;" << endl;
      defn << "      thr->callee_->scope = &" << mangle(net->name())
	   << "_scope;" << endl;
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

bool target_vvm::proc_wait(const NetEvWait*wait)
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
	    out << "static vvm_sync selector_" << id << ";" << endl;

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

      out << "static bool " << thread_class_ << "_step_" << out_step
	 << "_(vvm_thread*thr);" << endl;

      defn << "bool " << thread_class_ << "_step_" << out_step
	   << "_(vvm_thread*thr) {" << endl;

      return wait->emit_recurse(this);
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
void target_vvm::proc_while(const NetWhile*net)
{
      unsigned head_step = ++thread_step_;
      unsigned out_step = ++thread_step_;

      out << "static bool " << thread_class_ << "_step_"
	 << head_step << "_(vvm_thread*thr);" << endl;

      out << "static bool " << thread_class_ << "_step_"
	 << out_step << "_(vvm_thread*thr);" << endl;

      defn << "      thr->step_ = &" << thread_class_ << "_step_"
	   << head_step << "_;" << endl;
      defn << "      return true;" << endl;
      defn << "}" << endl;

      defn << "// " << net->expr()->get_line() <<
	    ": top of while condition." << endl;
      defn << "static bool " << thread_class_ << "_step_"
	   << head_step << "_(vvm_thread*thr) {" << endl;

      string expr = emit_proc_rval(this, net->expr());

      defn << "// " << net->expr()->get_line() <<
	    ": test while condition." << endl;
      if (net->expr()->expr_width() == 1) {
	    defn << "      if (!B_IS1(" << expr << "[0])) {" << endl;
      } else {
	    defn << "      if (!B_IS1(vvm_unop_or(" << expr << "))) {" << endl;
      }
      defn << "          thr->step_ = &" << thread_class_ << "_step_"
	   << out_step << "_;" << endl;
      defn << "          return true;" << endl;
      defn << "      }" << endl;

      net->emit_proc_recurse(this);

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
 * A delay suspends the thread for a period of time. If the delay
 * is an expression expresion, evaluate it at run time and use the
 * unsigned interpretation of it as the actual delay.
 */
bool target_vvm::proc_delay(const NetPDelay*proc)
{
      thread_step_ += 1;

      if (proc->expr()) {
	    string rval = emit_proc_rval(this, proc->expr());
	    defn << "      thr->step_ = &" << thread_class_ << "_step_"
		 << thread_step_ << "_;" << endl;
	    defn << "      thr->thread_yield(" << rval << ".as_unsigned());"
		 << endl;
	    defn << "      return false;" << endl;
	    defn << "}" << endl;

      } else {
	    defn << "      thr->step_ = &" << thread_class_ << "_step_"
		 << thread_step_ << "_;" << endl;
	    defn << "      thr->thread_yield(" << proc->delay() << ");"
		 << endl;
	    defn << "      return false;" << endl;
	    defn << "}" << endl;
      }

      out << "static bool " << thread_class_ << "_step_"
	 << thread_step_ << "_(vvm_thread*thr);" << endl;

      defn << "static bool " << thread_class_ << "_step_" << thread_step_
	   << "_(vvm_thread*thr) {" << endl;

      return proc->emit_proc_recurse(this);
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
 * Revision 1.206  2001/04/22 23:09:46  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.205  2001/04/06 02:28:02  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.204  2001/04/02 02:28:12  steve
 *  Generate code for task calls.
 *
 * Revision 1.203  2001/03/27 03:31:06  steve
 *  Support error code from target_t::end_design method.
 *
 * Revision 1.202  2001/02/13 04:11:24  steve
 *  Generate proper code for wide condition expressions.
 *
 * Revision 1.201  2001/01/16 03:57:46  steve
 *  Get rid of gate templates.
 *
 * Revision 1.200  2001/01/12 04:20:18  steve
 *  Generated function prototype. (PR#107)
 *
 * Revision 1.199  2001/01/06 22:22:17  steve
 *  Support signed decimal display of variables.
 *
 * Revision 1.198  2001/01/01 01:41:09  steve
 *  reg_assign into function ports. (PR#95)
 *
 * Revision 1.197  2000/12/17 05:33:11  steve
 *  Generate smaller code for reg assigns.
 *
 * Revision 1.196  2000/12/16 23:55:24  steve
 *  Generate loops to initialize vectors or constants.
 *
 * Revision 1.195  2000/12/16 16:57:43  steve
 *  Observe delays in non-blocking assignments (PR#83)
 *
 * Revision 1.194  2000/12/15 21:54:43  steve
 *  Allow non-blocking assign to pad memory word with zeros.
 *
 * Revision 1.193  2000/12/15 21:40:26  steve
 *  concatenation as parameter to system tasks. PR#64)
 *
 * Revision 1.192  2000/12/15 20:05:16  steve
 *  Fix memory access in vvm. (PR#70)
 *
 * Revision 1.191  2000/12/15 03:06:04  steve
 *  functions with system tasks (PR#46)
 *
 * Revision 1.190  2000/12/12 03:30:44  steve
 *  NetEUFuncs are allowed as system task parameters.
 *
 * Revision 1.189  2000/12/11 00:31:43  steve
 *  Add support for signed reg variables,
 *  simulate in t-vvm signed comparisons.
 *
 * Revision 1.188  2000/12/10 06:41:59  steve
 *  Support delays on continuous assignment from idents. (PR#40)
 *
 * Revision 1.187  2000/12/09 06:17:20  steve
 *  unary expressions as parameters (PR#42, PR#68)
 *
 * Revision 1.186  2000/11/30 17:31:42  steve
 *  Change LineInfo to store const C strings.
 *
 * Revision 1.185  2000/11/20 00:58:40  steve
 *  Add support for supply nets (PR#17)
 *
 * Revision 1.184  2000/11/11 01:52:09  steve
 *  change set for support of nmos, pmos, rnmos, rpmos, notif0, and notif1
 *  change set to correct behavior of bufif0 and bufif1
 *  (Tim Leight)
 *
 *  Also includes fix for PR#27
 *
 * Revision 1.183  2000/11/04 06:36:24  steve
 *  Apply sequential UDP rework from Stephan Boettcher  (PR#39)
 *
 * Revision 1.182  2000/10/29 17:10:02  steve
 *  task threads ned their scope initialized. (PR#32)
 *
 * Revision 1.181  2000/10/28 00:51:42  steve
 *  Add scope to threads in vvm, pass that scope
 *  to vpi sysTaskFunc objects, and add vpi calls
 *  to access that information.
 *
 *  $display displays scope in %m (PR#1)
 *
 * Revision 1.180  2000/10/26 00:29:10  steve
 *  Put signals into a signal_table
 *
 * Revision 1.179  2000/10/06 23:11:39  steve
 *  Replace data references with function calls. (Venkat)
 *
 * Revision 1.178  2000/10/06 02:21:35  steve
 *  sfuncs are char* and are compared with strcmp
 *
 * Revision 1.177  2000/09/26 01:35:42  steve
 *  Remove the obsolete NetEIdent class.
 *
 * Revision 1.176  2000/09/20 02:53:15  steve
 *  Correctly measure comples l-values of assignments.
 *
 * Revision 1.175  2000/09/17 21:26:15  steve
 *  Add support for modulus (Eric Aardoom)
 *
 * Revision 1.174  2000/09/16 21:28:14  steve
 *  full featured l-values for non-blocking assiginment.
 *
 * Revision 1.173  2000/09/10 02:18:16  steve
 *  elaborate complex l-values
 *
 * Revision 1.172  2000/09/08 17:08:10  steve
 *  initialize vlog info.
 *
 * Revision 1.171  2000/09/03 17:58:14  steve
 *  Properly ignore NetAssign_ objects.
 *
 * Revision 1.170  2000/09/02 20:54:21  steve
 *  Rearrange NetAssign to make NetAssign_ separate.
 *
 * Revision 1.169  2000/08/20 17:49:04  steve
 *  Clean up warnings and portability issues.
 *
 * Revision 1.168  2000/08/14 04:39:57  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.167  2000/08/09 03:43:45  steve
 *  Move all file manipulation out of target class.
 *
 * Revision 1.166  2000/08/08 01:50:42  steve
 *  target methods need not take a file stream.
 *
 * Revision 1.165  2000/08/02 00:57:02  steve
 *  tri01 support in vvm.
 *
 * Revision 1.164  2000/07/29 16:21:08  steve
 *  Report code generation errors through proc_delay.
 *
 * Revision 1.163  2000/07/26 03:53:11  steve
 *  Make simulation precision available to VPI.
 *
 * Revision 1.162  2000/07/14 06:12:57  steve
 *  Move inital value handling from NetNet to Nexus
 *  objects. This allows better propogation of inital
 *  values.
 *
 *  Clean up constant propagation  a bit to account
 *  for regs that are not really values.
 *
 * Revision 1.161  2000/07/07 04:53:54  steve
 *  Add support for non-constant delays in delay statements,
 *  Support evaluating ! in constant expressions, and
 *  move some code from netlist.cc to net_proc.cc.
 *
 * Revision 1.160  2000/06/25 19:59:42  steve
 *  Redesign Links to include the Nexus class that
 *  carries properties of the connected set of links.
 *
 * Revision 1.159  2000/06/24 16:40:46  steve
 *  expression scan uses tgt_ to get output files.
 *
 * Revision 1.158  2000/06/15 04:23:17  steve
 *  Binary expressions as operands to system tasks.
 *
 * Revision 1.157  2000/06/13 03:24:48  steve
 *  Index in memory assign should be a NetExpr.
 *
 * Revision 1.156  2000/06/06 02:32:45  steve
 *  Expand constants in its special case assignment. (Stephan Boettcher)
 */

