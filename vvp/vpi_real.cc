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
#ident "$Id: vpi_real.cc,v 1.9 2004/05/19 03:26:25 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  "schedule.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>


struct __vpiRealVar {
      struct __vpiHandle base;
      const char*name;
      double value;

      struct __vpiCallback*cb;
};

static char* real_var_get_str(int code, vpiHandle ref)
{
      assert(ref->vpi_type->type_code == vpiRealVar);

      struct __vpiRealVar*rfp = (struct __vpiRealVar*)ref;
      char *rbuf = need_result_buf(strlen(rfp->name) + 1, RBUF_STR);

      switch (code) {

	  case vpiName:
	    strcpy(rbuf, rfp->name);
	    return rbuf;

	  default:
	    return 0;
      }

      return 0;
}

static void real_var_get_value(vpiHandle ref, s_vpi_value*vp)
{
      assert(ref->vpi_type->type_code == vpiRealVar);

      struct __vpiRealVar*rfp = (struct __vpiRealVar*)ref;
      char*rbuf = need_result_buf(64 + 1, RBUF_VAL);

      switch (vp->format) {
	  case vpiObjTypeVal:
	    vp->format = vpiRealVal;

	  case vpiRealVal:
	    vp->value.real = rfp->value;
	    break;

	  case vpiIntVal:
	    vp->value.integer = (int)(rfp->value + 0.5);
	    break;

	  case vpiDecStrVal:
	    sprintf(rbuf, "%0.0f", rfp->value);
	    vp->value.str = rbuf;
	    break;

	  case vpiHexStrVal:
	    sprintf(rbuf, "%lx", (long)rfp->value);
	    vp->value.str = rbuf;
	    break;

	  case vpiBinStrVal: {
		unsigned long val = (unsigned long)rfp->value;
		unsigned len = 0;

		while (val > 0) {
		      len += 1;
		      val /= 2;
		}

		val = (unsigned long)rfp->value;
		for (unsigned idx = 0 ;  idx < len ;  idx += 1) {
		      rbuf[len-idx-1] = (val & 1)? '1' : '0';
		      val /= 2;
		}

		rbuf[len] = 0;
		if (len == 0) {
		      rbuf[0] = '0';
		      rbuf[1] = 0;
		}
		vp->value.str = rbuf;
		break;
	  }

	  default:
	    fprintf(stderr, "ERROR: Unsupported format code: %d\n",
		    vp->format);
	    assert(0);
      }
}

static vpiHandle real_var_put_value(vpiHandle ref, p_vpi_value vp)
{
      assert(ref->vpi_type->type_code == vpiRealVar);

      struct __vpiRealVar*rfp = (struct __vpiRealVar*)ref;

      assert(vp->format == vpiRealVal);
      rfp->value = vp->value.real;

      for (struct __vpiCallback*cur = rfp->cb ;  cur ;  cur = cur->next) {
	    cur->cb_data.time->type = vpiSimTime;
	    vpip_time_to_timestruct(cur->cb_data.time, schedule_simtime());

	    assert(cur->cb_data.cb_rtn != 0);

	    (cur->cb_data.cb_rtn)(&cur->cb_data);
      }

      return 0;
}

static const struct __vpirt vpip_real_var_rt = {
      vpiRealVar,

      0,
      &real_var_get_str,
      &real_var_get_value,
      &real_var_put_value,

      0,
      0,
      0,

      0
};

void vpip_real_value_change(struct __vpiCallback*cbh,
			     vpiHandle ref)
{
      struct __vpiRealVar*rfp = (struct __vpiRealVar*)ref;
      cbh->next = rfp->cb;
      rfp->cb = cbh;
}

vpiHandle vpip_make_real_var(const char*name)
{
      struct __vpiRealVar*obj = (struct __vpiRealVar*)
	    malloc(sizeof(struct __vpiRealVar));

      obj->base.vpi_type = &vpip_real_var_rt;
      obj->name = vpip_name_string(name);
      obj->value = 0.0;
      obj->cb = 0;

      return &obj->base;
}

/*
 * $Log: vpi_real.cc,v $
 * Revision 1.9  2004/05/19 03:26:25  steve
 *  Support delayed/non-blocking assignment to reals and others.
 *
 * Revision 1.8  2003/03/13 04:59:21  steve
 *  Use rbufs instead of static buffers.
 *
 * Revision 1.7  2003/03/06 04:32:00  steve
 *  Use hashed name strings for identifiers.
 *
 * Revision 1.6  2003/02/28 21:20:34  steve
 *  Allow read of realvar as int.
 *
 * Revision 1.5  2003/02/10 05:20:10  steve
 *  Add value change callbacks to real variables.
 *
 * Revision 1.4  2003/02/04 04:03:40  steve
 *  Add hex and binary formatting of real values.
 *
 * Revision 1.3  2003/02/02 01:40:24  steve
 *  Five vpi_free_object a default behavior.
 *
 * Revision 1.2  2003/01/27 00:14:37  steve
 *  Support in various contexts the $realtime
 *  system task.
 *
 * Revision 1.1  2003/01/25 23:48:06  steve
 *  Add thread word array, and add the instructions,
 *  %add/wr, %cmp/wr, %load/wr, %mul/wr and %set/wr.
 *
 */

