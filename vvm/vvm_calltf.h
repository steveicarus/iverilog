#ifndef __vvm_vvm_calltf_H
#define __vvm_vvm_calltf_H
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
#ident "$Id: vvm_calltf.h,v 1.1 1998/11/09 23:44:11 steve Exp $"
#endif

# include  "vvm.h"
# include  <string>

/*
 * The vvm environment supports external calls to C++ by
 * vvm_calltask. The code generator generates calls to vvm_calltask
 * that corresponds to the system call in the Verilog source. The
 * vvm_calltask in turn locates the function (by name) and calls the
 * C++ code that implements the task.
 *
 * The parameters of the task are implemented as an array of
 * vvm_calltf_parm objects. Each object represents a paramter from the
 * source.
 */
class vvm_calltf_parm {

    public:
      enum TYPE { NONE, ULONG, STRING, BITS, TIME };

      vvm_calltf_parm();
      explicit vvm_calltf_parm(TYPE);
      vvm_calltf_parm(const vvm_calltf_parm&);
      ~vvm_calltf_parm();

      TYPE type() const { return type_; }

      struct SIG {
	    vvm_bits_t*bits;
	    vvm_monitor_t*mon;
      };

      unsigned long as_ulong() const { return ulong_; }
      string as_string() const { return *(string*)string_; }
      vvm_bits_t* as_bits() const { return bits_.bits; }
      vvm_monitor_t* as_mon() const { return bits_.mon; }

      const string& sig_name() const { return bits_.mon->name(); }

      vvm_calltf_parm& operator= (unsigned long);
      vvm_calltf_parm& operator= (const string&);
      vvm_calltf_parm& operator= (const SIG&);
      vvm_calltf_parm& operator= (const vvm_calltf_parm&);

    private:
      TYPE type_;

      union {
	    unsigned long ulong_;
	    char string_[sizeof(string)];
	    SIG bits_;
      };

      void release_();
};

extern void vvm_calltask(vvm_simulation*sim, const string&name,
			 unsigned nparms, class vvm_calltf_parm*parms);

/*
 * $Log: vvm_calltf.h,v $
 * Revision 1.1  1998/11/09 23:44:11  steve
 *  Add vvm library.
 *
 */
#endif
