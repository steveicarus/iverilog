/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: t-null.cc,v 1.7 1999/09/30 21:27:29 steve Exp $"
#endif

# include  "netlist.h"
# include  "target.h"

/*
 * This target type simply drops the netlist. It is useful for
 * skipping the code generation phase.
 */
static class target_null_t  : public target_t {

    public:
      void bufz(ostream&os, const NetBUFZ*) { }
      void memory(ostream&, const NetMemory*) { }
      void task_def(ostream&, const NetTaskDef*) { }
      void net_esignal(ostream&, const NetESignal*) { }
      void net_event(ostream&, const NetNEvent*) { }
      bool proc_block(ostream&, const NetBlock*) { return true; }
      void proc_condit(ostream&, const NetCondit*) { }
      void proc_delay(ostream&, const NetPDelay*) { }
      void proc_event(ostream&, const NetPEvent*) { }
      void proc_forever(ostream&, const NetForever*) { }
      void proc_repeat(ostream&, const NetRepeat*) { }
      void proc_stask(ostream&, const NetSTask*) { }
      void proc_utask(ostream&os, const NetUTask*) { }

} target_null_obj;

extern const struct target tgt_null = { "null", &target_null_obj };
/*
 * $Log: t-null.cc,v $
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

