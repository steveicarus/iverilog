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
#ident "$Id: vvm_calltf.cc,v 1.2 1999/05/31 15:46:36 steve Exp $"
#endif

# include  "vvm_calltf.h"
# include  <new>
# include  <iostream>
# include  <assert.h>

vvm_calltf_parm::vvm_calltf_parm()
: type_(NONE)
{
}

vvm_calltf_parm::vvm_calltf_parm(TYPE t)
: type_(t)
{
      assert((t == NONE) || (t == TIME));
}

void vvm_calltf_parm::release_()
{
      switch (type_) {
	  case NONE:
	  case TIME:
	  case ULONG:
	    break;
	  case STRING:
	    ((string*)string_)->string::~string();
	    break;
	  case BITS:
	    break;
      }
      type_ = NONE;
}

vvm_calltf_parm& vvm_calltf_parm::operator= (unsigned long val)
{
      release_();
      type_ = ULONG;
      ulong_ = val;
      return *this;
}

vvm_calltf_parm& vvm_calltf_parm::operator= (const string&val)
{
      release_();
      type_ = STRING;
      new (string_) string (val);
      return *this;
}

vvm_calltf_parm& vvm_calltf_parm::operator= (const vvm_calltf_parm::SIG&val)
{
      release_();
      type_ = BITS;
      bits_ = val;
      return *this;
}

vvm_calltf_parm& vvm_calltf_parm::operator= (const vvm_calltf_parm&that)
{
      if (this == &that)
	    return *this;

      release_();
      switch (that.type_) {
	  case NONE:
	  case TIME:
	    type_ = that.type_;
	    break;
	  case ULONG:
	    type_ = ULONG;
	    ulong_ = that.ulong_;
	    break;
	  case STRING:
	    type_ = STRING;
	    new (string_) string (that.as_string());
	    break;
	  case BITS:
	    type_ = BITS;
	    bits_ = that.bits_;
	    break;
      }
      return *this;
}

vvm_calltf_parm::~vvm_calltf_parm()
{
      release_();
}

extern void Sdisplay(vvm_simulation*sim, const string&name,
		     unsigned nparms, class vvm_calltf_parm*parms);
extern void Smonitor(vvm_simulation*sim, const string&name,
		     unsigned nparms, class vvm_calltf_parm*parms);

static void Sfinish(vvm_simulation*sim, const string&,
		    unsigned, class vvm_calltf_parm*)
{
      sim->s_finish();
}

static struct {
      const string name;
      void (*func)(vvm_simulation*, const string&,
		   unsigned, class vvm_calltf_parm*);
} sys_table[] = {
      { "$display", &Sdisplay },
      { "$finish", &Sfinish },
      { "$monitor", &Smonitor },
      { "", 0 }
};

void vvm_calltask(vvm_simulation*sim, const string&fname,
		  unsigned nparms, class vvm_calltf_parm*parms)
{

      for (unsigned idx = 0 ;  sys_table[idx].func ;  idx += 1)
	    if (fname == sys_table[idx].name) {
		  sys_table[idx].func(sim, fname, nparms, parms);
		  return;
	    }

      cout << "Call " << fname << "(";
      for (unsigned idx = 0 ;  idx < nparms ;  idx += 1) {
	    if (idx > 0) cout << ", ";
	    switch (parms[idx].type()) {
		case vvm_calltf_parm::NONE:
		  break;
		case vvm_calltf_parm::ULONG:
		  cout << parms[idx].as_ulong();
		  break;
		case vvm_calltf_parm::STRING:
		  cout << "\"" << parms[idx].as_string() << "\"";
		  break;
		case vvm_calltf_parm::BITS:
		  cout << *parms[idx].as_bits();
		  break;
		case vvm_calltf_parm::TIME:
		  break;
	    }
      }
      cout << ")" << endl;
}

/*
 * $Log: vvm_calltf.cc,v $
 * Revision 1.2  1999/05/31 15:46:36  steve
 *  Handle time in more places.
 *
 * Revision 1.1  1998/11/09 23:44:10  steve
 *  Add vvm library.
 *
 */

