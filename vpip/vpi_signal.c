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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpi_signal.c,v 1.3 2002/08/12 01:35:06 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <assert.h>


static int signal_get(int code, vpiHandle ref)
{
      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;

      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      switch (code) {

	  case vpiSigned:
	    return rfp->signed_flag;
	  case vpiSize:
	    return rfp->nbits;

	  default:
	    return 0;
      }
}

static const char* signal_get_str(int code, vpiHandle ref)
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

      vpip_bits_get_value(rfp->bits, rfp->nbits, vp, rfp->signed_flag);
}

static vpiHandle signal_put_value(vpiHandle ref, s_vpi_value*vp,
			     p_vpi_time when, int flags)
{
      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      vpip_bits_set_value(rfp->bits, rfp->nbits, vp);
      return ref;
}

static const struct __vpirt vpip_net_rt = {
      vpiNet,
      signal_get,
      signal_get_str,
      signal_get_value,
      signal_put_value,
      0,
      0
};

vpiHandle vpip_make_net(struct __vpiSignal*ref, const char*name,
			vpip_bit_t*b, unsigned nb, int signed_flag)
{
      ref->base.vpi_type = &vpip_net_rt;
      ref->name = name;
      ref->bits = b;
      ref->nbits = nb;
      ref->signed_flag = signed_flag? 1 : 0;
      ref->mfirst = 0;
      ref->mlast  = 0;
      return &(ref->base);
}

static const struct __vpirt vpip_reg_rt = {
      vpiReg,
      signal_get,
      signal_get_str,
      signal_get_value,
      signal_put_value,
      0,
      0
};

vpiHandle vpip_make_reg(struct __vpiSignal*ref, const char*name,
			vpip_bit_t*b, unsigned nb, int signed_flag)
{
      ref->base.vpi_type = &vpip_reg_rt;
      ref->name = name;
      ref->bits = b;
      ref->nbits = nb;
      ref->signed_flag = signed_flag? 1 : 0;
      ref->mfirst = 0;
      ref->mlast  = 0;
      return &(ref->base);
}

/*
 * $Log: vpi_signal.c,v $
 * Revision 1.3  2002/08/12 01:35:06  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/10/26 02:29:10  steve
 *  const/non-const warnings. (Stephan Boettcher)
 *
 * Revision 1.1  2001/03/14 19:27:44  steve
 *  Rearrange VPI support libraries.
 *
 * Revision 1.12  2001/01/06 22:22:17  steve
 *  Support signed decimal display of variables.
 *
 * Revision 1.11  2000/08/20 17:49:05  steve
 *  Clean up warnings and portability issues.
 *
 * Revision 1.10  2000/05/18 03:27:32  steve
 *  Support writing scalars and vectors to signals.
 *
 * Revision 1.9  2000/03/31 07:08:39  steve
 *  allow cancelling of cbValueChange events.
 *
 * Revision 1.8  2000/03/25 05:02:25  steve
 *  signal bits are referenced at run time by the vpiSignal struct.
 *
 * Revision 1.7  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.6  1999/12/15 04:01:14  steve
 *  Add the VPI implementation of $readmemh.
 *
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

