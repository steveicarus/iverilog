/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: t-null.cc,v 1.16 2000/08/14 04:39:57 steve Exp $"
#endif

# include  "netlist.h"
# include  "target.h"

/*
 * This target type simply drops the netlist. It is useful for
 * skipping the code generation phase.
 */
static class target_null_t  : public target_t {

    public:
      bool start_design(const Design*) { return true; }
      bool bufz(const NetBUFZ*) { return true; }
      void event(const NetEvent*) { }
      void func_def(const NetFuncDef*) { }
      void memory(const NetMemory*) { }
      void task_def(const NetTaskDef*) { }
      void net_assign(const NetAssign*) { }
      void net_assign_nb(const NetAssignNB*) { }
      bool net_const(const NetConst*) { return true; }
      void net_probe(const NetEvProbe*) { }
      bool proc_block(const NetBlock*) { return true; }
      void proc_condit(const NetCondit*) { }
      bool proc_delay(const NetPDelay*) { return true; }
      void proc_forever(const NetForever*) { }
      void proc_repeat(const NetRepeat*) { }
      void proc_stask(const NetSTask*) { }
      void proc_utask(const NetUTask*) { }
      bool proc_wait(const NetEvWait*) { return true; }

} target_null_obj;

extern const struct target tgt_null = { "null", &target_null_obj };
/*
 * $Log: t-null.cc,v $
 * Revision 1.16  2000/08/14 04:39:57  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.15  2000/08/09 03:43:45  steve
 *  Move all file manipulation out of target class.
 *
 * Revision 1.14  2000/08/08 01:50:42  steve
 *  target methods need not take a file stream.
 *
 * Revision 1.13  2000/07/29 16:21:08  steve
 *  Report code generation errors through proc_delay.
 *
 * Revision 1.12  2000/06/14 20:29:39  steve
 *  Ignore more things in null target.
 *
 * Revision 1.11  2000/04/12 04:23:58  steve
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
 * Revision 1.10  2000/02/23 02:56:55  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.9  2000/02/14 06:04:52  steve
 *  Unary reduction operators do not set their operand width
 *
 * Revision 1.8  1999/10/05 03:26:37  steve
 *  null target ignore assignment nodes.
 *
 * Revision 1.7  1999/09/30 21:27:29  steve
 *  Ignore user task definitions.
 *
 * Revision 1.6  1999/09/22 16:57:24  steve
 *  Catch parallel blocks in vvm emit.
 *
 * Revision 1.5  1999/09/17 02:06:26  steve
 *  Handle unconnected module ports.
 *
 * Revision 1.4  1999/07/03 02:12:52  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.3  1999/06/19 21:06:16  steve
 *  Elaborate and supprort to vvm the forever
 *  and repeat statements.
 *
 * Revision 1.2  1999/06/06 20:33:30  steve
 *  implement some null-target code generation.
 *
 * Revision 1.1  1999/01/24 01:35:08  steve
 *  Support null target for generating no output.
 *
 */

