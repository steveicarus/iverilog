/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: a_vcl.c,v 1.4 2003/04/30 01:09:29 steve Exp $"
#endif

#include  <vpi_user.h>
#include  <acc_user.h>
#include  <stdlib.h>
#ifdef HAVE_MALLOC_H
#include  <malloc.h>
#endif
#include  "priv.h"

struct vcl_record {
      vpiHandle obj;
      PLI_INT32(*consumer)(p_vc_record);
      void*user_data;
      PLI_INT32 vcl_flag;

      vpiHandle callback;
      struct vcl_record*next;
};

static struct vcl_record*vcl_list = 0;

static PLI_INT32 vcl_value_callback(struct t_cb_data*cb)
{
      struct vcl_record*cur = (struct vcl_record*)cb->user_data;

      vpi_printf("XXXX Call vcl_callback(<type=%d>);\n",
		 vpi_get(vpiType, cur->obj));

      return 0;
}

void acc_vcl_add(handle obj, PLI_INT32(*consumer)(p_vc_record),
		 void*data, PLI_INT32 vcl_flag)
{
      struct vcl_record*cur;
      struct t_cb_data cb;

      switch (vpi_get(vpiType, obj)) {
	  case vpiNet:
	    cur = malloc(sizeof (struct vcl_record));
	    cur->obj = obj;
	    cur->consumer = consumer;
	    cur->user_data = data;
	    cur->vcl_flag = vcl_flag;
	    cur->next = vcl_list;
	    vcl_list = cur;

	    cb.reason = cbValueChange;
	    cb.cb_rtn = vcl_value_callback;
	    cb.obj = obj;
	    cb.user_data = (void*)cur;
	    cur->callback = vpi_register_cb(&cb);
	    break;

	  default:
	    vpi_printf("XXXX Call acc_vcl_add(<type=%d>, ..., %d);\n",
		       vpi_get(vpiType, obj), vcl_flag);
	    break;
      }

}

void acc_vcl_delete(handle obj, PLI_INT32(*consumer)(p_vc_record),
		    void*data, PLI_INT32 vcl_flag)
{
      vpi_printf("XXXX Call acc_vcl_delete\n");
}


/*
 * $Log: a_vcl.c,v $
 * Revision 1.4  2003/04/30 01:09:29  steve
 *  Conditionally include malloc.h
 *
 * Revision 1.3  2003/04/24 02:02:37  steve
 *  Clean up some simple warnings.
 *
 * Revision 1.2  2003/04/20 02:48:39  steve
 *  Support value change callbacks.
 *
 * Revision 1.1  2003/04/12 18:57:14  steve
 *  More acc_ function stubs.
 *
 */

