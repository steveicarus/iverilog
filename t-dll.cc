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
#ident "$Id: t-dll.cc,v 1.8 2000/09/24 15:46:00 steve Exp $"
#endif

# include  "compiler.h"
# include  "t-dll.h"
# include  <dlfcn.h>

static struct dll_target dll_target_obj;

bool dll_target::start_design(const Design*des)
{
      dll_path_ = des->get_flag("DLL");
      dll_ = dlopen(dll_path_.c_str(), RTLD_NOW);
      if (dll_ == 0) {
	    cerr << dll_path_ << ": " << dlerror() << endl;
	    return false;
      }

      stmt_cur_ = 0;

      des_ = (ivl_design_t)des;

      start_design_ = (start_design_f)dlsym(dll_, "target_start_design");
      end_design_   = (end_design_f)  dlsym(dll_, "target_end_design");

      net_bufz_   = (net_bufz_f)  dlsym(dll_, LU "target_net_bufz" TU);
      net_const_  = (net_const_f) dlsym(dll_, LU "target_net_const" TU);
      net_event_  = (net_event_f) dlsym(dll_, LU "target_net_event" TU);
      net_logic_  = (net_logic_f) dlsym(dll_, LU "target_net_logic" TU);
      net_probe_  = (net_probe_f) dlsym(dll_, LU "target_net_probe" TU);
      net_signal_ = (net_signal_f)dlsym(dll_, LU "target_net_signal" TU);
      process_    = (process_f)   dlsym(dll_, LU "target_process" TU);
      scope_      = (scope_f)     dlsym(dll_, LU "target_scope" TU);

      (start_design_)(des_);
      return true;
}

void dll_target::end_design(const Design*)
{
      (end_design_)(des_);
      dlclose(dll_);
}

bool dll_target::bufz(const NetBUFZ*net)
{
      if (net_bufz_) {
	    int rc = (net_bufz_)(net->name(), 0);
	    return rc == 0;

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_bufz function." << endl;
	    return false;
      }

      return false;
}

void dll_target::event(const NetEvent*net)
{
      if (net_event_) {
	    (net_event_)(net->full_name().c_str(), 0);

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_event function." << endl;
      }

      return;
}

void dll_target::logic(const NetLogic*net)
{
      struct ivl_net_logic_s obj;
      obj.dev_ = net;

      if (net_logic_) {
	    (net_logic_)(net->name(), &obj);

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_logic function." << endl;
      }

      return;
}

bool dll_target::net_const(const NetConst*net)
{
      struct ivl_net_const_s obj;

      obj.con_ = net;

      if (net_const_) {
	    int rc = (net_const_)(net->name(), &obj);
	    return rc == 0;

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_const function." << endl;
	    return false;
      }

      return false;
}

void dll_target::net_probe(const NetEvProbe*net)
{
      if (net_probe_) {
	    int rc = (net_probe_)(net->name(), 0);
	    return;

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_probe function." << endl;
	    return;
      }

      return;
}

void dll_target::scope(const NetScope*net)
{
      if (scope_)
	    (scope_)( (ivl_scope_t)net );
}

void dll_target::signal(const NetNet*net)
{
      if (net_signal_) {
	    int rc = (net_signal_)(net->name(), (ivl_signal_t)net);
	    return;

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_signal function." << endl;
	    return;
      }
}

extern const struct target tgt_dll = { "dll", &dll_target_obj };


/*
 * $Log: t-dll.cc,v $
 * Revision 1.8  2000/09/24 15:46:00  steve
 *  API access to signal type and port type.
 *
 * Revision 1.7  2000/09/18 01:24:32  steve
 *  Get the structure for ivl_statement_t worked out.
 *
 * Revision 1.6  2000/08/27 15:51:51  steve
 *  t-dll iterates signals, and passes them to the
 *  target module.
 *
 *  Some of NetObj should return char*, not string.
 *
 * Revision 1.5  2000/08/26 00:54:03  steve
 *  Get at gate information for ivl_target interface.
 *
 * Revision 1.4  2000/08/20 04:13:57  steve
 *  Add ivl_target support for logic gates, and
 *  make the interface more accessible.
 *
 * Revision 1.3  2000/08/19 18:12:42  steve
 *  Add target calls for scope, events and logic.
 *
 * Revision 1.2  2000/08/14 04:39:57  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.1  2000/08/12 16:34:37  steve
 *  Start stub for loadable targets.
 *
 */

