#ifndef __pform_H
#define __pform_H
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
#ident "$Id: pform.h,v 1.1 1998/11/03 23:29:04 steve Exp $"
#endif

# include  "netlist.h"
# include  "Module.h"
# include  "Statement.h"
# include  "PGate.h"
# include  "PExpr.h"
# include  "PWire.h"
# include  "verinum.h"
# include  <iostream.h>
# include  <string>
# include  <list>
# include  <vector>
# include  <stdio.h>

/*
 * These classes implement the parsed form (P-form for short) of the
 * original verilog source. the parser generates the pform for the
 * convenience of later processing steps.
 */


/*
 * Wire objects represent the named wires (of various flavor) declared
 * in the source.
 *
 * Gate objects are the functional modules that are connected together
 * by wires.
 *
 * Wires and gates, connected by joints, represent a netlist. The
 * netlist is therefore a representation of the desired circuit.
 */
class PGate;
class PExpr;

/*
 * These type are lexical types -- that is, types that are used as
 * lexical values to decorate the parse tree during parsing. They are
 * not in any way preserved once parsing is done.
 */

struct lgate {
      string name;
      list<PExpr*>*parms;
};


/*
 * The parser uses startmodule and endmodule together to build up a
 * module as it parses it. The startmodule tells the pform code that a
 * module has been noticed in the source file and the following events
 * are to apply to the scope of that module. The endmodule causes the
 * pform to close up and finish the named module.
 */
extern void pform_startmodule(const string&, list<PWire*>*ports);
extern void pform_endmodule(const string&);

/*
 * The makewire functions announce to the pform code new wires. These
 * go into a module that is currently opened.
 */
extern void pform_makewire(const string&name, NetNet::Type type);
extern void pform_makewire(const list<string>*names, NetNet::Type type);
extern void pform_set_port_type(list<string>*names, NetNet::PortType);
extern void pform_set_net_range(list<string>*names, list<PExpr*>*);
extern void pform_make_behavior(PProcess::Type, Statement*);
extern Statement* pform_make_block(PBlock::BL_TYPE, list<Statement*>*);
extern Statement* pform_make_assignment(string*t, PExpr*e);
extern Statement* pform_make_calltask(string*t, list<PExpr*>* =0);

/*
 * The makegate function creates a new gate (which need not have a
 * name) and connects it to the specified wires.
 */
extern void pform_makegate(PGBuiltin::Type type,
			   const string&name,
			   const vector<string>&wires,
			   unsigned long delay_value);

extern void pform_makegates(PGBuiltin::Type type,
			    PExpr*delay,
			    list<lgate>*gates);

extern void pform_make_modgates(const string&type, list<lgate>*gates);

/* Make a continuous assignment node, with optional bit- or part- select. */
extern void pform_make_pgassign(const string&lval, PExpr*rval);
extern void pform_make_pgassign(const string&lval, PExpr*sel, PExpr*rval);

/*
 * These are functions that the outside-the-parser code uses the do
 * interesting things to the verilog. The parse function reads and
 * parses the source file and places all the modules it finds into the
 * mod list. The dump function dumps a module to the output stream.
 */
extern int  pform_parse(FILE*, list<Module*>&mod);
extern void pform_dump(ostream&out, Module*mod);

/*
 * $Log: pform.h,v $
 * Revision 1.1  1998/11/03 23:29:04  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
