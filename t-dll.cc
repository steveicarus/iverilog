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
#ident "$Id: t-dll.cc,v 1.1 2000/08/12 16:34:37 steve Exp $"
#endif

# include  "target.h"
# include  "ivl_target.h"
# include  <dlfcn.h>

struct ivl_design_s {
      const Design*des_;
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

      void*dll_;

      struct ivl_design_s ivl_des;

      start_design_f start_design_;
      end_design_f   end_design_;

} dll_target_obj;


bool dll_target::start_design(const Design*des)
{
      dll_ = dlopen(des->get_flag("DLL").c_str(), RTLD_NOW);
      if (dll_ == 0) {
	    cerr << des->get_flag("DLL") << ": " << dlerror() << endl;
	    return false;
      }

      ivl_des.des_ = des;

      start_design_ = (start_design_f)dlsym(dll_, "target_start_design");
      end_design_   = (end_design_f)  dlsym(dll_, "target_end_design");
      (start_design_)(&ivl_des);
      return true;
}

void dll_target::end_design(const Design*)
{
      (end_design_)(&ivl_des);
      dlclose(dll_);
}

extern const struct target tgt_dll = { "dll", &dll_target_obj };


/* THE FOLLOWING ARE FUNCTIONS THAT ARE CALLED FROM THE TARGET. */

extern "C" const char*ivl_get_flag(ivl_design_t des, const char*key)
{
      return des->des_->get_flag(key).c_str();
}

/*
 * $Log: t-dll.cc,v $
 * Revision 1.1  2000/08/12 16:34:37  steve
 *  Start stub for loadable targets.
 *
 */

