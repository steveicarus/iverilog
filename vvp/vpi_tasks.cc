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
#if !defined(WINNT)
#ident "$Id: vpi_tasks.cc,v 1.1 2001/03/16 01:44:34 steve Exp $"
#endif

/*
 * This file keeps the table of system/task definitions. This table is
 * built up before the input source file is parsed, and is used by the
 * compiler when %vpi_call statements are encountered.
 */
# include "vpi_priv.h"
# include  <stdio.h>
# include  <malloc.h>
# include  <string.h>
# include  <assert.h>

static vpiHandle systask_handle(int type, vpiHandle ref)
{
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

#if 0
      return vpip_make_iterator(rfp->nargs, rfp->args);
#else
      return 0;
#endif
}

static const struct __vpirt vpip_systask_rt = {
      vpiSysTaskCall,
      0,
      0,
      0,
      0,
      systask_handle,
      systask_iter
};


/*
 * A value *can* be put to a vpiSysFuncCall object. This is how the
 * return value is set. The value that is given should be converted to
 * bits and set into the return value bit array.
 */
static vpiHandle sysfunc_put_value(vpiHandle ref, p_vpi_value val,
				   p_vpi_time t, int flag)
{
      assert(0);
      return 0;
}

static const struct __vpirt vpip_sysfunc_rt = {
      vpiSysFuncCall,
      0,
      0,
      0,
      sysfunc_put_value,
      0,
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
vpiHandle vpip_build_vpi_call(const char*name)
{
      struct __vpiSysTaskCall*obj = (struct __vpiSysTaskCall*)
	    calloc(1, sizeof (struct __vpiSysTaskCall));

      obj->base.vpi_type = &vpip_systask_rt;
      obj->defn = vpip_find_systf(name);
      obj->nargs = 0;
      obj->args = 0;

	/* If there is a compiletf function, call it here. */
      if (obj->defn->info.compiletf)
	    obj->defn->info.compiletf (obj->defn->info.user_data);

      return &obj->base;
}


/*
 * This function is used by the %vpi_call instruction to actually
 * place the call to the system task/function. For now, only support
 * calls to system tasks.
 */
void vpip_execute_vpi_call(vpiHandle ref)
{
      assert(ref->vpi_type->type_code == vpiSysTaskCall);

      vpip_cur_task = (struct __vpiSysTaskCall*)ref;

      assert(vpip_cur_task->defn->info.calltf);
      vpip_cur_task->defn->info.calltf (vpip_cur_task->defn->info.user_data);
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
      cur->info = *ss;
      cur->info.tfname = strdup(ss->tfname);
}

/*
 * $Log: vpi_tasks.cc,v $
 * Revision 1.1  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 */

