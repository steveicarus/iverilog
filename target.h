#ifndef IVL_target_H
#define IVL_target_H
/*
 * Copyright (c) 1998-2021 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "netlist.h"

/*
 * This header file describes the types and constants used to describe
 * the possible target output types of the compiler. The backends
 * provide one of these in order to tell the previous steps what the
 * backend is able to do.
 */

/*
 * The backend driver is hooked into the compiler, and given a name,
 * by creating an instance of the target structure. The structure has
 * the name that the compiler will use to locate the driver, and a
 * pointer to a target_t object that is the actual driver.
 */
struct target {
      const char* name;
      struct target_t* meth;
};

/*
 * The emit process uses a target_t driver to send the completed
 * design to a file. It is up to the driver object to follow along in
 * the iteration through the design, generating output as it can.
 */

struct target_t {
      inline target_t() : errors(0) { }
      virtual ~target_t();

	/* Set this to count errors encountered during emit. */
      int errors;

	/* Start the design. This sets the main output file stream
	   that the target should use. */
      virtual bool start_design(const Design*) =0;

	/* This is called once for each scope in the design, before
	   anything else is called. */
      virtual void scope(const NetScope*);

      virtual bool class_type(const NetScope*, netclass_t*);

	/* This is called to convert module ports from a NetNet* to an
	 * ivl_signal_t object. */
      virtual void convert_module_ports(const NetScope*);

	/* Output an event object. Called for each named event in the scope. */
      virtual void event(const NetEvent*);

        /* Output an enumeration typespec. */
      virtual bool enumeration(const NetScope*, netenum_t*);

	/* Output a signal (called for each signal) */
      virtual void signal(const NetNet*) =0;
      virtual bool signal_paths(const NetNet*);

        /* Analog branches */
      virtual bool branch(const NetBranch*);

	/* Output a defined task. */
      virtual void task_def(const NetScope*);
      virtual bool func_def(const NetScope*);

	/* LPM style components are handled here. */
      virtual void lpm_abs(const NetAbs*);
      virtual void lpm_add_sub(const NetAddSub*);
      virtual bool lpm_array_dq(const NetArrayDq*);
      virtual void lpm_clshift(const NetCLShift*);
      virtual bool lpm_cast_int2(const NetCastInt2*);
      virtual bool lpm_cast_int4(const NetCastInt4*);
      virtual bool lpm_cast_real(const NetCastReal*);
      virtual void lpm_compare(const NetCompare*);
      virtual void lpm_divide(const NetDivide*);
      virtual void lpm_modulo(const NetModulo*);
      virtual void lpm_ff(const NetFF*);
      virtual void lpm_latch(const NetLatch*);
      virtual void lpm_mult(const NetMult*);
      virtual void lpm_mux(const NetMux*);
      virtual void lpm_pow(const NetPow*);

      virtual bool concat(const NetConcat*);
      virtual bool part_select(const NetPartSelect*);
      virtual bool replicate(const NetReplicate*);

	/* Output a gate (called for each gate) */
      virtual void logic(const NetLogic*);
      virtual bool tran(const NetTran*);
      virtual bool ureduce(const NetUReduce*); /* unary reduction operator */
      virtual bool bufz(const NetBUFZ*);
      virtual void udp(const NetUDP*);
      virtual void net_case_cmp(const NetCaseCmp*);
      virtual bool net_const(const NetConst*);
      virtual bool net_sysfunction(const NetSysFunc*);
      virtual bool net_function(const NetUserFunc*);
      virtual bool net_literal(const NetLiteral*);
      virtual void net_probe(const NetEvProbe*);
      virtual bool sign_extend(const NetSignExtend*);
      virtual bool substitute(const NetSubstitute*);

	/* Output a process (called for each process). It is up to the
	   target to recurse if desired. */
      virtual bool process(const NetProcTop*);
      virtual bool process(const NetAnalogTop*);

	/* Various kinds of process nodes are dispatched through these. */
      virtual void proc_alloc(const NetAlloc*);
      virtual bool proc_assign(const NetAssign*);
      virtual void proc_assign_nb(const NetAssignNB*);
      virtual bool proc_block(const NetBlock*);
      virtual bool proc_break(const NetBreak*);
      virtual void proc_case(const NetCase*);
      virtual bool proc_cassign(const NetCAssign*);
      virtual bool proc_condit(const NetCondit*);
      virtual bool proc_continue(const NetContinue*);
      virtual bool proc_contribution(const NetContribution*);
      virtual bool proc_deassign(const NetDeassign*);
      virtual bool proc_delay(const NetPDelay*);
      virtual bool proc_disable(const NetDisable*);
      virtual void proc_do_while(const NetDoWhile*);
      virtual bool proc_force(const NetForce*);
      virtual bool proc_forloop(const NetForLoop*) =0;
      virtual void proc_forever(const NetForever*);
      virtual void proc_free(const NetFree*);
      virtual bool proc_release(const NetRelease*);
      virtual void proc_repeat(const NetRepeat*);
      virtual bool proc_trigger(const NetEvTrig*);
      virtual bool proc_nb_trigger(const NetEvNBTrig*);
      virtual void proc_stask(const NetSTask*);
      virtual void proc_utask(const NetUTask*);
      virtual bool proc_wait(const NetEvWait*);
      virtual void proc_while(const NetWhile*);

	/* Done with the design. The target returns !0 if there is
	   some error in the code generation. */
      virtual int end_design(const Design*);
};

/* This class is used by the NetExpr class to help with the scanning
   of expressions. */
struct expr_scan_t {
      virtual ~expr_scan_t();
      virtual void expr_access_func(const NetEAccess*);
      virtual void expr_array_pattern(const NetEArrayPattern*);
      virtual void expr_const(const NetEConst*);
      virtual void expr_last(const NetELast*);
      virtual void expr_new(const NetENew*);
      virtual void expr_null(const NetENull*);
      virtual void expr_param(const NetEConstParam*);
      virtual void expr_property(const NetEProperty*);
      virtual void expr_rparam(const NetECRealParam*);
      virtual void expr_creal(const NetECReal*);
      virtual void expr_concat(const NetEConcat*);
      virtual void expr_event(const NetEEvent*);
      virtual void expr_scope(const NetEScope*);
      virtual void expr_scopy(const NetEShallowCopy*);
      virtual void expr_select(const NetESelect*);
      virtual void expr_sfunc(const NetESFunc*);
      virtual void expr_signal(const NetESignal*);
      virtual void expr_ternary(const NetETernary*);
      virtual void expr_ufunc(const NetEUFunc*);
      virtual void expr_unary(const NetEUnary*);
      virtual void expr_binary(const NetEBinary*);
      virtual void expr_netenum(const NetENetenum*);
};


/* This function takes a fully qualified Verilog name (which may have,
   for example, dots in it) and produces a mangled version that can be
   used by most any language. */
extern std::string mangle(const std::string&str);

/* This function takes a string and produces an escaped version that can be
   used inside a string constant for a C++ compiler. */
extern std::string stresc(const std::string&str);

#endif /* IVL_target_H */
