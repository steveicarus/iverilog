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
#ident "$Id: vpi_signal.c,v 1.5 1999/11/07 20:33:30 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <assert.h>


static int signal_get(int code, vpiHandle ref)
{
      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;

      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      switch (code) {
	  case vpiSize:
	    return rfp->nbits;

	  default:
	    return 0;
      }
}

static char* signal_get_str(int code, vpiHandle ref)
{
      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;

      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      switch (code) {

	  case vpiFullName:
	    return (char*)rfp->name;
      }

      return 0;
}

static void signal_get_value(vpiHandle ref, s_vpi_value*vp)
{
      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      vpip_bits_get_value(rfp->bits, rfp->nbits, vp);
}

static const struct __vpirt vpip_net_rt = {
      vpiNet,
      signal_get,
      signal_get_str,
      signal_get_value,
      0,
      0
};

vpiHandle vpip_make_net(struct __vpiSignal*ref, const char*name)
{
      ref->base.vpi_type = &vpip_net_rt;
      ref->name = name;
      ref->monitor = 0;
      return &(ref->base);
}

static const struct __vpirt vpip_reg_rt = {
      vpiReg,
      signal_get,
      signal_get_str,
      signal_get_value,
      0,
      0
};

vpiHandle vpip_make_reg(struct __vpiSignal*ref, const char*name)
{
      ref->base.vpi_type = &vpip_reg_rt;
      ref->name = name;
      ref->monitor = 0;
      return &(ref->base);
}

/*
 * $Log: vpi_signal.c,v $
 * Revision 1.5  1999/11/07 20:33:30  steve
 *  Add VCD output and related system tasks.
 *
 * Revision 1.4  1999/11/07 02:25:08  steve
 *  Add the $monitor implementation.
 *
 * Revision 1.3  1999/11/06 16:52:16  steve
 *  complete value retrieval for number constants.
 *
 * Revision 1.2  1999/10/29 03:37:22  steve
 *  Support vpiValueChance callbacks.
 *
 * Revision 1.1  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 */

