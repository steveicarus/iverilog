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
#ident "$Id: t-dll.cc,v 1.4 2000/08/20 04:13:57 steve Exp $"
#endif

# include  "target.h"
# include  "ivl_target.h"
# include  "compiler.h"
# include  <dlfcn.h>

struct ivl_design_s {
      const Design*des_;
};

struct ivl_net_const_s {
      const NetConst*con_;
};

struct ivl_net_logic_s {
      const NetLogic*dev_;
};

struct ivl_process_s {
      const NetProcTop*top_;
};

struct ivl_scope_s {
      const NetScope*scope_;
};

/*
 * The DLL target type loads a named object file to handle the process
 * of scanning the netlist. When it is time to start the design, I
 * locate and link in the desired DLL, then start calling methods. The
 * DLL will call me back to get information out of the netlist in
 * particular.
 */
struct dll_target  : public target_t {

      bool start_design(const Design*);
      void end_design(const Design*);

      bool bufz(const NetBUFZ*);
      void event(const NetEvent*);
      void logic(const NetLogic*);
      bool net_const(const NetConst*);
      void net_probe(const NetEvProbe*);

      bool process(const NetProcTop*);
      void scope(const NetScope*);

      void*dll_;
      string dll_path_;

      struct ivl_design_s ivl_des;

      start_design_f start_design_;
      end_design_f   end_design_;

      net_bufz_f   net_bufz_;
      net_const_f  net_const_;
      net_event_f  net_event_;
      net_logic_f  net_logic_;
      net_probe_f  net_probe_;

      process_f    process_;
      scope_f      scope_;

} dll_target_obj;


bool dll_target::start_design(const Design*des)
{
      dll_path_ = des->get_flag("DLL");
      dll_ = dlopen(dll_path_.c_str(), RTLD_NOW);
      if (dll_ == 0) {
	    cerr << dll_path_ << ": " << dlerror() << endl;
	    return false;
      }

      ivl_des.des_ = des;

      start_design_ = (start_design_f)dlsym(dll_, "target_start_design");
      end_design_   = (end_design_f)  dlsym(dll_, "target_end_design");

      net_bufz_   = (net_bufz_f)  dlsym(dll_, LU "target_net_bufz" TU);
      net_const_  = (net_const_f) dlsym(dll_, LU "target_net_const" TU);
      net_event_  = (net_event_f) dlsym(dll_, LU "target_net_event" TU);
      net_logic_  = (net_logic_f) dlsym(dll_, LU "target_net_logic" TU);
      net_probe_  = (net_probe_f) dlsym(dll_, LU "target_net_probe" TU);
      process_    = (process_f)   dlsym(dll_, LU "target_process" TU);
      scope_      = (scope_f)     dlsym(dll_, LU "target_scope" TU);

      (start_design_)(&ivl_des);
      return true;
}

void dll_target::end_design(const Design*)
{
      (end_design_)(&ivl_des);
      dlclose(dll_);
}

bool dll_target::bufz(const NetBUFZ*net)
{
      if (net_bufz_) {
	    int rc = (net_bufz_)(net->name().c_str(), 0);
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
	    (net_logic_)(net->name().c_str(), &obj);

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
	    int rc = (net_const_)(net->name().c_str(), &obj);
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
	    int rc = (net_probe_)(net->name().c_str(), 0);
	    return;

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_probe function." << endl;
	    return;
      }

      return;
}

bool dll_target::process(const NetProcTop*net)
{
      struct ivl_process_s obj;

      obj.top_ = net;

      if (process_) {
	    int rc = (process_)(&obj);
	    return rc == 0;

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_process function." << endl;
	    return false;
      }

      return false;
}

void dll_target::scope(const NetScope*net)
{
      struct ivl_scope_s obj;

      obj.scope_ = net;

      if (scope_)
	    (scope_)(&obj);
}

extern const struct target tgt_dll = { "dll", &dll_target_obj };


/* THE FOLLOWING ARE FUNCTIONS THAT ARE CALLED FROM THE TARGET. */

extern "C" const char*ivl_get_flag(ivl_design_t des, const char*key)
{
      return des->des_->get_flag(key).c_str();
}

extern "C" ivl_logic_t ivl_get_logic_type(ivl_net_logic_t net)
{
      switch (net->dev_->type()) {
	  case NetLogic::AND:
	    return IVL_AND;
	  case NetLogic::OR:
	    return IVL_OR;
      }
      assert(0);
      return IVL_AND;
}

/*
 * $Log: t-dll.cc,v $
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

