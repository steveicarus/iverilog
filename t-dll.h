#ifndef __t_dll_H
#define __t_dll_H
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: t-dll.h,v 1.8 2000/10/05 05:03:01 steve Exp $"
#endif

# include  "target.h"
# include  "ivl_target.h"

struct ivl_design_s {
      ivl_scope_t root_;
      const Design*self;
};

/*
 * The DLL target type loads a named object file to handle the process
 * of scanning the netlist. When it is time to start the design, I
 * locate and link in the desired DLL, then start calling methods. The
 * DLL will call me back to get information out of the netlist in
 * particular.
 */
struct dll_target  : public target_t, public expr_scan_t {

      bool start_design(const Design*);
      void end_design(const Design*);

      bool bufz(const NetBUFZ*);
      void event(const NetEvent*);
      void logic(const NetLogic*);
      bool net_const(const NetConst*);
      void net_probe(const NetEvProbe*);

      bool process(const NetProcTop*);
      void scope(const NetScope*);
      void signal(const NetNet*);

      void*dll_;
      string dll_path_;

      ivl_design_s des_;

      start_design_f start_design_;
      end_design_f   end_design_;

      net_bufz_f   net_bufz_;
      net_const_f  net_const_;
      net_event_f  net_event_;
      net_logic_f  net_logic_;
      net_probe_f  net_probe_;
      net_signal_f net_signal_;

      process_f    process_;
      scope_f      scope_;

	/* These methods and members are used for forming the
	   statements of a thread. */
      struct ivl_statement_s*stmt_cur_;
      void proc_assign(const NetAssign*);
      bool proc_block(const NetBlock*);
      void proc_condit(const NetCondit*);
      bool proc_delay(const NetPDelay*);
      void proc_stask(const NetSTask*);
      bool proc_trigger(const NetEvTrig*);
      bool proc_wait(const NetEvWait*);
      void proc_while(const NetWhile*);

      struct ivl_expr_s*expr_;
      void expr_binary(const NetEBinary*);
      void expr_const(const NetEConst*);
      void expr_sfunc(const NetESFunc*);
      void expr_signal(const NetESignal*);
};

/*
 * These are various private declarations used by the t-dll target.
 */

/*
 * The ivl_expr_t is an opaque reference to one of these
 * structures. This structure holds all the information we need about
 * an expression node, including its type, the expression width, and
 * type specific properties.
 */
struct ivl_expr_s {
      ivl_expr_type_t type_;

      unsigned width_  :24;
      unsigned signed_ : 1;

      union {
	    struct {
		  char op_;
		  ivl_expr_t lef_;
		  ivl_expr_t rig_;
	    } binary_;

	    struct {
		  char*bits_;
	    } number_;

	    struct {
		  char*name_;
	    } sfunc_;

	    struct {
		  char*value_;
	    } string_;

	    struct {
		  char*name_;
		  ivl_expr_t msb_;
		  ivl_expr_t lsb_;
	    } subsig_;
      } u_;
};

struct ivl_net_const_s {
      unsigned width_  :24;
      unsigned signed_ : 1;
      char *bits_;
};

struct ivl_net_logic_s {
      const NetLogic*dev_;
};

struct ivl_process_s {
      ivl_process_type_t type_;
      ivl_statement_t stmt_;
};

struct ivl_scope_s {
      ivl_scope_t child_, sibling_;

      const NetScope*self;
};

/*
 * The ivl_statement_t represents any statement. The type of statement
 * is defined by the ivl_statement_type_t enumeration. Given the type,
 * certain information about the statement may be available.
 */
struct ivl_statement_s {
      enum ivl_statement_type_e type_;
      union {
	    struct { /* IVL_ST_BLOCK */
		  struct ivl_statement_s*stmt_;
		  unsigned nstmt_;
	    } block_;

	    struct { /* IVL_ST_CONDIT */
		    /* This is the condition expression */
		  ivl_expr_t cond_;
		    /* This is two statements, the true and false. */
		  struct ivl_statement_s*stmt_;
	    } condit_;

	    struct { /* IVL_ST_DELAY */
		  unsigned long delay_;
		  ivl_statement_t stmt_;
	    } delay_;

	    struct { /* IVL_ST_DELAYX */
		  int expr_; /* XXXX */
		  ivl_statement_t stmt_;
	    } delayx_;

	    struct { /* IVL_ST_STASK */
		  char* name_;
		  unsigned nparm_;
		  ivl_expr_t*parms_;
	    } stask_;

	    struct { /* IVL_ST_TRIGGER */
		  ivl_net_event_t event_;
	    } trig_;

	    struct { /* IVL_ST_WAIT */
		  int cond_; /* XXXX */
		  ivl_statement_t stmt_;
	    } wait_;

	    struct { /* IVL_ST_WHILE */
		  int cond_; /* XXXX */
		  ivl_statement_t stmt_;
	    } while_;
      } u_;
};

/*
 * $Log: t-dll.h,v $
 * Revision 1.8  2000/10/05 05:03:01  steve
 *  xor and constant devices.
 *
 * Revision 1.7  2000/09/30 02:18:15  steve
 *  ivl_expr_t support for binary operators,
 *  Create a proper ivl_scope_t object.
 *
 * Revision 1.6  2000/09/26 00:30:07  steve
 *  Add EX_NUMBER and ST_TRIGGER to dll-api.
 *
 * Revision 1.5  2000/09/24 02:21:53  steve
 *  Add support for signal expressions.
 *
 * Revision 1.4  2000/09/23 05:15:07  steve
 *  Add enough tgt-verilog code to support hello world.
 *
 * Revision 1.3  2000/09/22 03:58:30  steve
 *  Access to the name of a system task call.
 *
 * Revision 1.2  2000/09/19 04:15:27  steve
 *  Introduce the means to get statement types.
 *
 * Revision 1.1  2000/09/18 01:24:32  steve
 *  Get the structure for ivl_statement_t worked out.
 *
 */
#endif
