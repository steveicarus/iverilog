/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_callback.c,v 1.1 1999/10/28 00:47:25 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <stdlib.h>
# include  <assert.h>

static struct __vpirt vpip_callback_rt = {
      vpiCallback,
      0,
      0,
      0,
      0,
      0
};


static void vpip_call_callback(void*cp)
{
      struct __vpiCallback*rfp = (struct __vpiCallback*)cp;
      assert(rfp->base.vpi_type->type_code == vpiCallback);
      rfp->cb_data.cb_rtn(&rfp->cb_data);
      free(rfp);
}

/*
 * XXXX This currently does not support (or pay any attention to) the
 * delay parameter. Yikes!
 */
vpiHandle vpi_register_cb(p_cb_data data)
{
      struct __vpiCallback*rfp = calloc(1, sizeof(struct  __vpiCallback));
      rfp->base.vpi_type = &vpip_callback_rt;
      rfp->cb_data = *data;

      switch (data->reason) {
	  case cbReadOnlySynch:
	    assert(data->time);
	    assert(data->time->type == vpiSimTime);
	    assert(data->time->low == 0);
	    assert(data->time->high == 0);
	    rfp->ev = vpip_sim_insert_event(0, rfp, vpip_call_callback, 1);
	    break;

	  default:
	    assert(0);
      }

      return &(rfp->base);
}


int vpi_remove_cb(vpiHandle ref)
{
      struct __vpiCallback*rfp = (struct __vpiCallback*)ref;
      assert(ref->vpi_type->type_code == vpiCallback);
      vpip_sim_cancel_event(rfp->ev);
      free(rfp);
      return 0;
}

/*
 * $Log: vpi_callback.c,v $
 * Revision 1.1  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 */

