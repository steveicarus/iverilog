/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvm_calltf.cc,v 1.6 1999/09/29 01:41:18 steve Exp $"
#endif

# include  "vvm_calltf.h"
# include  <vpi_user.h>
# include  "vpi_priv.h"
# include  <new>
# include  <iostream>
# include  <assert.h>
# include  <stdlib.h>
# include  <stdarg.h>
# include  <malloc.h>
# include  <stdio.h>
# include  <dlfcn.h>

# define MAX_PATHLEN 1024

/* This simulation pointer is used by vpi functions to get back to the
   simulation. */
static vvm_simulation*vpi_sim;

static vpiHandle vvm_vpi_cur_task;

/*
 * Keep a list of vpi_systf_data structures. This list is searched
 * forward whenever a function is invoked by name, and items are
 * pushed in front of the list whenever they are registered. This
 * allows entries to override older entries.
 */
struct systf_entry {
      struct systf_entry* next;
      s_vpi_systf_data systf_data;
};

static struct systf_entry*systf_list = 0;

extern "C" void vpi_register_systf(const struct t_vpi_systf_data*systf)
{
      struct systf_entry*cur = new struct systf_entry;
      cur->systf_data = *systf;
      cur->systf_data.tfname = strdup(systf->tfname);
      cur->next = systf_list;
      systf_list = cur;
}

extern "C" void vpi_printf(const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      vprintf(fmt, ap);
      va_end(ap);
}

extern "C" void vpi_sim_control(int func, ...)
{
      switch (func) {
	  case vpiFinish:
	    vpi_sim->s_finish();
	    break;
      }
}

extern "C" vpiHandle vpi_handle(int type, vpiHandle ref)
{
      switch (type) {
	  case vpiSysTfCall:
	    return vvm_vpi_cur_task;

	  default:
	    return 0;
      }
}

extern "C" vpiHandle vpi_iterate(int type, vpiHandle ref)
{
      vpiHandle res = (vpiHandle)calloc(1, sizeof (struct __vpiHandle));
      res->type = type;
      res->referent = ref;
      res->val.unum = 0;

      return res;
}

extern "C" vpiHandle vpi_scan(vpiHandle ref)
{
      assert(ref->type == vpiArgument);
      if (ref->val.unum >= ref->referent->narguments) {
	    vpi_free_object(ref);
	    return 0;
      }

      return ref->referent->arguments[ref->val.unum++];
}

extern "C" int vpi_get(int property, vpiHandle ref)
{
      if (property == vpiType)
	    return ref->type;

      if (ref->get_ == 0)
	    return -1;

      return (ref->get_)(property, ref);
}

extern "C" char* vpi_get_str(int property, vpiHandle ref)
{
      if (property == vpiFullName)
	    return ref->full_name;

      if (ref->get_str_ == 0)
	    return 0;

      return (ref->get_str_)(property, ref);

      switch (property) {
	  case vpiName:
	    return ref->full_name;
      }
      return 0;
}

extern "C" void vpi_get_value(vpiHandle expr, s_vpi_value*vp)
{
      if (expr->get_value_) {
	    expr->get_value_(expr, vp);
	    return;
      }

      vp->format = vpiSuppressVal;
}

extern "C" int vpi_free_object(vpiHandle ref)
{
      free(ref);
      return 0;
}

/*
 * This function is a get_value_ method of a vpiHandle, that supports
 * reading bits as a string.
 */
static void get_value_bits(vpiHandle ref, s_vpi_value*vp)
{
      static char buff[1024];
      char*cp;
      unsigned width, bytes;
      unsigned val;
      assert(ref->val.bits);
      width = ref->val.bits->get_width();
      cp = buff;

      switch (vp->format) {
	  case vpiObjTypeVal:
	  case vpiBinStrVal:
	    for (unsigned idx = 0 ;  idx < width ;  idx += 1)
		  switch (ref->val.bits->get_bit(width-idx-1)) {
		      case V0:
			*cp++ = '0';
			break;
		      case V1:
			*cp++ = '1';
			break;
		      case Vx:
			*cp++ = 'x';
			break;
		      case Vz:
			*cp++ = 'z';
			break;
		  }
	    vp->format = vpiBinStrVal;
	    break;

	  case vpiDecStrVal:
	    val = 0;
	    for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
		  val *= 2;
		  switch (ref->val.bits->get_bit(width-idx-1)) {
		      case V0:
		      case Vx:
		      case Vz:
			break;
		      case V1:
			val += 1;
			break;
		  }
	    }
	    sprintf(cp, "%u", val);
	    cp += strlen(cp);
	    break;

	  case vpiOctStrVal:
	    bytes = width%3;
	    if (bytes) {
		  *cp++ = '?';
	    }
	    for (unsigned idx = bytes ;  idx < width ;  idx += 3) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  for (unsigned i = idx ;  i < idx+3 ;  i += 1) {
			v *= 2;
			switch (ref->val.bits->get_bit(width-idx-i-1)) {
			    case V0:
			      break;
			    case V1:
			      v += 1;
			      break;
			    case Vx:
			      x += 1;
			      break;
			    case Vz:
			      z += 1;
			      break;
			}
		  }
		  if (x == 3)
			*cp++ = 'x';
		  else if (x > 0)
			*cp++ = 'X';
		  else if (z == 3)
			*cp++ = 'z';
		  else if (z > 0)
			*cp++ = 'Z';
		  else
			*cp++ = "01234567"[v];
	    }
	    break;

	  case vpiHexStrVal:
	    if (width%4) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  for (unsigned i = 0 ;  i < width%4 ;  i += 1) {
			v *= 2;
			switch (ref->val.bits->get_bit(width-i-1)) {
			    case V0:
			      break;
			    case V1:
			      v += 1;
			      break;
			    case Vx:
			      x += 1;
			      break;
			    case Vz:
			      z += 1;
			      break;
			}
		  }
		  if (x == width%4)
			*cp++ = 'x';
		  else if (x > 0)
			*cp++ = 'X';
		  else if (z == width%4)
			*cp++ = 'z';
		  else if (z > 0)
			*cp++ = 'Z';
		  else
			*cp++ = "0123456789abcdef"[v];
	    }

	    for (unsigned idx = width%4 ;  idx < width ;  idx += 4) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  for (unsigned i = idx ;  i < idx+4 ;  i += 1) {
			v *= 2;
			switch (ref->val.bits->get_bit(width-i-1)) {
			    case V0:
			      break;
			    case V1:
			      v += 1;
			      break;
			    case Vx:
			      x += 1;
			      break;
			    case Vz:
			      z += 1;
			      break;
			}
		  }
		  if (x == 4)
			*cp++ = 'x';
		  else if (x > 0)
			*cp++ = 'X';
		  else if (z == 4)
			*cp++ = 'z';
		  else if (z > 0)
			*cp++ = 'Z';
		  else
			*cp++ = "0123456789abcdef"[v];
	    }
	    break;

	  default:
	    *cp++ = '(';
	    *cp++ = '?';
	    *cp++ = ')';
	    break;
      }

      *cp++ = 0;
      vp->value.str = buff;
}

static void get_value_strconst(vpiHandle ref, s_vpi_value*vp)
{
      switch (vp->format) {
	  case vpiObjTypeVal:
	  case vpiStringVal:
	    vp->value.str = ref->full_name;
	    vp->format = vpiStringVal;
	    break;

	  default:
	    vp->format = vpiSuppressVal;
	    break;
      }
}

static void get_value_timevar(vpiHandle ref, s_vpi_value*vp)
{
      static char buf[128];
      switch (vp->format) {
	  case vpiObjTypeVal:
	  case vpiTimeVal:
	    vp->value.time = &ref->val.time;
	    vp->format = vpiTimeVal;
	    break;

	  case vpiDecStrVal:
	    sprintf(buf, "%u", ref->val.time.low);
	    vp->value.str = buf;
	    break;

	  default:
	    vp->format = vpiSuppressVal;
	    vp->value.str = 0;
	    break;
      }
}

/*
 * The load_vpi_module function attempts to locate and load the named
 * vpi module and call the included startup routines. This is invoked
 * by the generated C++ code to load all the modules that the
 * simulation requires.
 *
 * If there is a '/' character in the name, or there is no
 * VPI_MODULE_PATH, the the name is usd as is. No path is searched for
 * the module.
 *
 * If there is a VPI_MODULE_PATH and there is no '/' in the name, the
 * VPI_MODULE_PATH is taken as a ':' separated list of directory
 * names. Each directory is searched for a module with the right name
 * that will link in. The current working directory is not implicitly
 * tried. If you wish '.' be in th search path, include it.
 */
typedef void (*vlog_startup_routines_t)(void);

void vvm_load_vpi_module(const char*name)
{
      void*mod = 0;
      const char*path = getenv("VPI_MODULE_PATH");

      if ((path == 0) || (strchr(name, '/'))) {
	  mod = dlopen(name, RTLD_NOW);
	  if (mod == 0) {
		cerr << name << ": " << dlerror() << endl;
		return;
	  }

      } else {
	    const char*cur = path;
	    const char*ep;
	    for (cur = path ; cur  ; cur = ep? ep+1 : 0) {
		  char dest[MAX_PATHLEN+1];

		  ep = strchr(cur, ':');
		  size_t n = ep? ep-cur : strlen(cur);
		  if ((n + strlen(name) + 2) > sizeof dest)
			continue;

		  strncpy(dest, cur, n);
		  dest[n] = '/';
		  dest[n+1] = 0;
		  strcat(dest, name);

		  mod = dlopen(dest, RTLD_NOW);
		  if (mod) break;
	    }
      }

      if (mod == 0) {
	    cerr << dlerror() << endl;
	    return;
      }

      void*table = dlsym(mod, "vlog_startup_routines");
      vlog_startup_routines_t*routines = (vlog_startup_routines_t*)table;
      if (routines == 0) {
	    cerr << name << ": Unable to locate the vlog_startup_routines"
		 " table." << endl;
	    dlclose(mod);
	    return;
      }

      for (unsigned idx = 0 ;  routines[idx] ;  idx += 1)
	    (routines[idx])();
}

void vvm_make_vpi_parm(vpiHandle ref, const char*val)
{
      memset(ref, 0, sizeof*ref);
      ref->type = vpiConstant;
      ref->subtype = vpiStringConst;
      ref->full_name = const_cast<char*>(val);
      ref->get_value_ = &get_value_strconst;
}

void vvm_make_vpi_parm(vpiHandle ref, vvm_bits_t*val)
{
      memset(ref, 0, sizeof*ref);
      ref->type = vpiConstant;
      ref->subtype = vpiBinaryConst;
      ref->full_name = "";
      ref->val.bits = val;
      ref->get_value_ = &get_value_bits;
}

void vvm_init_vpi_handle(vpiHandle ref, vvm_bits_t*bits, vvm_monitor_t*mon)
{
      memset(ref, 0, sizeof*ref);
      ref->type = vpiReg;
      ref->full_name = const_cast<char*>(mon->name());
      ref->val.bits = bits;
      ref->get_value_ = &get_value_bits;
}

void vvm_init_vpi_timevar(vpiHandle ref, const char*name)
{
      memset(ref, 0, sizeof*ref);
      ref->type = vpiTimeVar;
      ref->full_name = const_cast<char*>(name);
      ref->get_value_ = &get_value_timevar;
}

void vvm_make_vpi_parm(vpiHandle ref)
{
      memset(ref, 0, sizeof*ref);
      ref->type = 0;
}

void vvm_calltask(vvm_simulation*sim, const string&fname,
		  unsigned nparms, vpiHandle*parms)
{
      vpi_sim = sim;

      struct __vpiHandle cur_task;
      cur_task.type = vpiSysTaskCall;
      cur_task.full_name = 0;
      cur_task.referent = 0;
      cur_task.arguments = parms;
      cur_task.narguments = nparms;

      vvm_vpi_cur_task = &cur_task;

	/* Look for a systf function to invoke. */
      for (systf_entry*idx = systf_list ;  idx ;  idx = idx->next)
	    if (fname == idx->systf_data.tfname) {
		  cur_task.full_name = idx->systf_data.tfname;
		  idx->systf_data.calltf(idx->systf_data.user_data);
		  return;
	    }


	/* Finally, if nothing is found then something is not
	   right. Print out the function name all the parameters
	   passed, so that someone can deal with it. */
      cout << "Call " << fname << endl;
}


/*
 * $Log: vvm_calltf.cc,v $
 * Revision 1.6  1999/09/29 01:41:18  steve
 *  Support the $write system task, and have the
 *  vpi_scan function free iterators as needed.
 *
 * Revision 1.5  1999/09/13 03:08:10  steve
 *  fix vpiHexStrVal dumping of digits to strings.
 *
 * Revision 1.4  1999/08/19 02:51:11  steve
 *  Add vpi_sim_control
 *
 * Revision 1.3  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 * Revision 1.2  1999/05/31 15:46:36  steve
 *  Handle time in more places.
 *
 * Revision 1.1  1998/11/09 23:44:10  steve
 *  Add vvm library.
 *
 */

