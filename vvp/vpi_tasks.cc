/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpi_tasks.cc,v 1.27 2004/05/19 03:30:46 steve Exp $"
#endif

/*
 * This file keeps the table of system/task definitions. This table is
 * built up before the input source file is parsed, and is used by the
 * compiler when %vpi_call statements are encountered.
 */
# include  "vpi_priv.h"
# include  "vthread.h"
# include  <stdio.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

static const struct __vpirt vpip_systask_def_rt = {
      vpiSysTask,
      0,
      0,
      0,
      0,
      0,
      0
};

static const struct __vpirt vpip_sysfunc_def_rt = {
      vpiSysFunc,
      0,
      0,
      0,
      0,
      0,
      0
};

static vpiHandle systask_handle(int type, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;
      assert((ref->vpi_type->type_code == vpiSysTaskCall)
	     || (ref->vpi_type->type_code == vpiSysFuncCall));

      switch (type) {
	  case vpiScope:
	    return &rfp->scope->base;
	  default:
	    return 0;
      };
}

static int systask_get(int type, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;

      assert((ref->vpi_type->type_code == vpiSysTaskCall)
	     || (ref->vpi_type->type_code == vpiSysFuncCall));

      switch (type) {
	  case vpiTimeUnit:
	    return rfp->scope->time_units;
	  default:
	    return vpiUndefined;
      }
}

/*
 * the get_str function only needs to support vpiName
 */

static char *systask_get_str(int type, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;

      assert((ref->vpi_type->type_code == vpiSysTaskCall)
	     || (ref->vpi_type->type_code == vpiSysFuncCall));

      const char *bn = rfp->defn->info.tfname;
      char *rbuf = need_result_buf(strlen(bn) + 1, RBUF_STR);

      switch (type) {

          case vpiName:
            strcpy(rbuf,bn);
	    return rbuf;
      }

      return 0;
}

/*
 * the iter function only supports getting an iterator of the
 * arguments. This works equally well for tasks and functions.
 */
static vpiHandle systask_iter(int type, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;
      assert((ref->vpi_type->type_code == vpiSysTaskCall)
	     || (ref->vpi_type->type_code == vpiSysFuncCall));

      if (rfp->nargs == 0)
	    return 0;

      return vpip_make_iterator(rfp->nargs, rfp->args, false);
}

static const struct __vpirt vpip_systask_rt = {
      vpiSysTaskCall,
      systask_get,
      systask_get_str,
      0,
      0,
      systask_handle,
      systask_iter
};


/*
 * A value *can* be put to a vpiSysFuncCall object. This is how the
 * return value is set. The value that is given should be converted to
 * bits and set into the thread space bits that were selected at
 * compile time.
 */
static vpiHandle sysfunc_put_value(vpiHandle ref, p_vpi_value vp)
{
      assert(ref->vpi_type->type_code == vpiSysFuncCall);

      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;

      assert(rfp->vbit >= 4);

      switch (vp->format) {
	    
	  case vpiIntVal: {
		long val = vp->value.integer;
		for (int idx = 0 ;  idx < rfp->vwid ;  idx += 1) {
		      vthread_put_bit(vpip_current_vthread,
				      rfp->vbit+idx, val&1);
		      val >>= 1;
		}
		break;
	  }

	  case vpiTimeVal:
		for (int idx = 0 ;  idx < rfp->vwid ;  idx += 1) {
		      PLI_INT32 word;
		      if (idx >= 32)
			    word = vp->value.time->high;
		      else
			    word = vp->value.time->low;

		      word >>= idx % 32;

		      vthread_put_bit(vpip_current_vthread,
				      rfp->vbit+idx, word&1);
		}
		break;


	  case vpiScalarVal:
	    switch (vp->value.scalar) {
		case vpi0:
		  vthread_put_bit(vpip_current_vthread, rfp->vbit, 0);
		  break;
		case vpi1:
		  vthread_put_bit(vpip_current_vthread, rfp->vbit, 1);
		  break;
		case vpiX:
		  vthread_put_bit(vpip_current_vthread, rfp->vbit, 2);
		  break;
		case vpiZ:
		  vthread_put_bit(vpip_current_vthread, rfp->vbit, 3);
		  break;
		default:
		  assert(0);
	    }
	    break;

	  case vpiVectorVal:

	    for (unsigned wdx = 0 ;  wdx < (unsigned)rfp->vwid ;  wdx += 32) {
		  unsigned word = wdx / 32;
		  unsigned long aval = vp->value.vector[word].aval;
		  unsigned long bval = vp->value.vector[word].bval;

		  for (unsigned idx = 0 ;  (wdx+idx) < (unsigned)rfp->vwid ;
		      idx += 1)
		  {
			int bit = (aval&1) | ((bval<<1)&2);

			switch (bit) {
			    case 0:
			      bit = 0;
			      break;
			    case 1:
			      bit = 1;
			      break;
			    case 2:
			      bit = 3;
			      break;
			    case 3:
			      bit = 2;
			      break;
			}
			vthread_put_bit(vpip_current_vthread,
					rfp->vbit+wdx+idx, bit);

			aval >>= 1;
			bval >>= 1;
		  }
	    }
	    break;

	  default:
	    assert(0);
      }

      return 0;
}

static vpiHandle sysfunc_put_real_value(vpiHandle ref, p_vpi_value vp)
{
      assert(ref->vpi_type->type_code == vpiSysFuncCall);

      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;

	/* Make sure this is a real valued function. */
      assert(rfp->vwid == -vpiRealConst);

      double val = 0.0;

      switch (vp->format) {

	  case vpiRealVal:
	    val = vp->value.real;
	    break;

	  default:
	    assert(0);
      }

      vthread_put_real(vpip_current_vthread, rfp->vbit, val);
      return 0;
}


static const struct __vpirt vpip_sysfunc_rt = {
      vpiSysFuncCall,
      0,
      systask_get_str,
      0,
      sysfunc_put_value,
      systask_handle,
      systask_iter
};

static const struct __vpirt vpip_sysfunc_real_rt = {
      vpiSysFuncCall,
      0,
      systask_get_str,
      0,
      sysfunc_put_real_value,
      systask_handle,
      systask_iter
};

  /* **** Manipulate the internal datastructures. **** */

static struct __vpiUserSystf**def_table = 0;
static unsigned def_count = 0;

static struct __vpiUserSystf* allocate_def(void)
{
      if (def_table == 0) {
	    def_table = (struct __vpiUserSystf**)
		  malloc(sizeof (struct __vpiUserSystf*));

	    def_table[0] = (struct __vpiUserSystf*)
		  calloc(1, sizeof(struct __vpiUserSystf));

	    def_count = 1;
	    return def_table[0];
      }

      def_table = (struct __vpiUserSystf**)
	    realloc(def_table, (def_count+1)*sizeof (struct __vpiUserSystf*));

      def_table[def_count] = (struct __vpiUserSystf*)
	    calloc(1, sizeof(struct __vpiUserSystf));

      return def_table[def_count++];
}


static struct __vpiUserSystf* vpip_find_systf(const char*name)
{
      for (unsigned idx = 0 ;  idx < def_count ;  idx += 1)
	    if (strcmp(def_table[idx]->info.tfname, name) == 0)
		  return def_table[idx];

      return 0;
}

/*
 * A vpi_call is actually built up into a vpiSysTaskCall VPI object
 * that refers back to the vpiUserSystf VPI object that is the
 * definition. So this function is called by the compiler when a
 * %vpi_call statement is encountered. Create here a vpiHandle that
 * describes the call, and return it. The %vpi_call instruction will
 * store this handle for when it is executed.
 */
vpiHandle vpip_build_vpi_call(const char*name, unsigned vbit, int vwid,
			      unsigned argc, vpiHandle*argv)
{
      struct __vpiUserSystf*defn = vpip_find_systf(name);
      if (defn == 0) {
	    fprintf(stderr, "%s: This task not defined "
		    "by any modules. I cannot compile it.\n", name);
	    return 0;
      }

      switch (defn->info.type) {
	  case vpiSysTask:
	    if (vwid != 0) {
		  fprintf(stderr, "%s: This is a system Task, "
			  "you cannot call it as a Function\n", name);
		  return 0;
	    }
	    assert(vbit == 0);
	    break;

	  case vpiSysFunc:
	    if (vwid == 0) {
		  fprintf(stderr, "%s: This is a system Function, "
			  "you cannot call it as a Task\n", name);
		  return 0;
	    }
	    break;

	  default:
	    assert(0);
      }

      struct __vpiSysTaskCall*obj = new struct __vpiSysTaskCall;

      switch (defn->info.type) {
	  case vpiSysTask:
	    obj->base.vpi_type = &vpip_systask_rt;
	    break;
	    
	  case vpiSysFunc:
	    if (vwid > 0) {
		  obj->base.vpi_type = &vpip_sysfunc_rt;

	    } else switch (vwid) {

		case -vpiRealConst:
		  obj->base.vpi_type = &vpip_sysfunc_real_rt;
		  break;

		default:
		  assert(0);
		  obj->base.vpi_type = &vpip_sysfunc_rt;
	    }
	    break;
      }

      obj->scope = vpip_peek_current_scope();
      obj->defn  = defn;
      obj->nargs = argc;
      obj->args  = argv;
      obj->vbit  = vbit;
      obj->vwid  = vwid;
      obj->userdata = 0;

	/* If there is a compiletf function, call it here. */
      if (obj->defn->info.compiletf) {

	    assert(vpi_mode_flag == VPI_MODE_NONE);
	    vpi_mode_flag = VPI_MODE_COMPILETF;
	    vpip_cur_task = obj;
	    obj->defn->info.compiletf (obj->defn->info.user_data);
	    vpip_cur_task = 0;
	    vpi_mode_flag = VPI_MODE_NONE;
      }

      return &obj->base;
}


/*
 * This function is used by the %vpi_call instruction to actually
 * place the call to the system task/function. For now, only support
 * calls to system tasks.
 */

vthread_t vpip_current_vthread;

void vpip_execute_vpi_call(vthread_t thr, vpiHandle ref)
{
      vpip_current_vthread = thr;

      assert((ref->vpi_type->type_code == vpiSysTaskCall)
	     || (ref->vpi_type->type_code == vpiSysFuncCall));

      vpip_cur_task = (struct __vpiSysTaskCall*)ref;

      if (vpip_cur_task->defn->info.calltf) {
	    assert(vpi_mode_flag == VPI_MODE_NONE);
	    vpi_mode_flag = VPI_MODE_CALLTF;
	    vpip_cur_task->defn->info.calltf(vpip_cur_task->defn->info.user_data);
	    vpi_mode_flag = VPI_MODE_NONE;
      }
}

/*
 * This is the entry function that a VPI module uses to hook a new
 * task/function into the simulator. The function creates a new
 * __vpi_userSystf to represent the definition for the calls that come
 * to pass later.
 */
void vpi_register_systf(const struct t_vpi_systf_data*ss)
{
      struct __vpiUserSystf*cur = allocate_def();
      assert(ss);
      switch (ss->type) {
	  case vpiSysTask:
	    cur->base.vpi_type = &vpip_systask_def_rt;
	    break;
	  case vpiSysFunc:
	    cur->base.vpi_type = &vpip_sysfunc_def_rt;
	    break;
	  default:
	    assert(0);
      }

      cur->info = *ss;
      cur->info.tfname = strdup(ss->tfname);
}

PLI_INT32 vpi_put_userdata(vpiHandle ref, void*data)
{
      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;
      assert((ref->vpi_type->type_code == vpiSysTaskCall)
	     || (ref->vpi_type->type_code == vpiSysFuncCall));

      rfp->userdata = data;
      return 0;
}

void* vpi_get_userdata(vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;
      assert((ref->vpi_type->type_code == vpiSysTaskCall)
	     || (ref->vpi_type->type_code == vpiSysFuncCall));

      return rfp->userdata;
}


/*
 * $Log: vpi_tasks.cc,v $
 * Revision 1.27  2004/05/19 03:30:46  steve
 *  Support delayed/non-blocking assignment to reals and others.
 *
 * Revision 1.26  2004/02/18 02:51:59  steve
 *  Fix type mismatches of various VPI functions.
 *
 * Revision 1.25  2003/12/07 20:05:56  steve
 *  Ducument lxt2 access.
 *
 * Revision 1.24  2003/06/18 00:54:28  steve
 *  Account for all 64 bits in results of $time.
 *
 * Revision 1.23  2003/03/14 05:00:44  steve
 *  Support vpi_get of vpiTimeUnit.
 *
 * Revision 1.22  2003/03/13 05:07:10  steve
 *  signed/unsigned warnings.
 *
 * Revision 1.21  2003/03/07 02:44:14  steve
 *  Support vector put of function return values.
 *
 * Revision 1.20  2003/03/03 03:27:35  steve
 *  Support vpiName for system task/function calls.
 *
 * Revision 1.19  2003/01/27 00:14:37  steve
 *  Support in various contexts the $realtime
 *  system task.
 *
 * Revision 1.18  2003/01/09 04:09:44  steve
 *  Add vpi_put_userdata
 *
 * Revision 1.17  2002/12/21 00:55:58  steve
 *  The $time system task returns the integer time
 *  scaled to the local units. Change the internal
 *  implementation of vpiSystemTime the $time functions
 *  to properly account for this. Also add $simtime
 *  to get the simulation time.
 *
 * Revision 1.16  2002/08/12 01:35:09  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.15  2002/06/02 19:05:50  steve
 *  Check for null pointers from users.
 *
 * Revision 1.14  2002/05/18 02:34:11  steve
 *  Add vpi support for named events.
 *
 *  Add vpi_mode_flag to track the mode of the
 *  vpi engine. This is for error checking.
 *
 * Revision 1.13  2002/05/09 03:34:31  steve
 *  Handle null time and calltf pointers.
 *
 * Revision 1.12  2002/05/03 15:44:11  steve
 *  Add vpiModule iterator to vpiScope objects.
 *
 * Revision 1.11  2002/04/07 02:34:10  steve
 *  Set vpip_cur_task while calling compileft
 *
 * Revision 1.10  2001/09/15 18:27:05  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.9  2001/08/03 06:50:44  steve
 *  Detect system function used as a task.
 *
 * Revision 1.8  2001/06/25 03:12:06  steve
 *  Give task/function definitions a vpi type object.
 *
 * Revision 1.7  2001/05/20 00:46:12  steve
 *  Add support for system function calls.
 *
 * Revision 1.6  2001/05/10 00:26:53  steve
 *  VVP support for memories in expressions,
 *  including general support for thread bit
 *  vectors as system task parameters.
 *  (Stephan Boettcher)
 *
 * Revision 1.5  2001/04/18 04:21:23  steve
 *  Put threads into scopes.
 *
 * Revision 1.4  2001/03/22 22:38:14  steve
 *  Detect undefined system tasks at compile time.
 *
 * Revision 1.3  2001/03/18 04:35:18  steve
 *  Add support for string constants to VPI.
 *
 * Revision 1.2  2001/03/18 00:37:55  steve
 *  Add support for vpi scopes.
 *
 * Revision 1.1  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 */

