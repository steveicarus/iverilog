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
#ident "$Id: vpi_signal.c,v 1.2 1999/10/29 03:37:22 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <assert.h>
# include  <string.h>
# include  <stdio.h>


static char* signal_get_str(int code, vpiHandle ref)
{
      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;

      switch (code) {

	  case vpiFullName:
	    return (char*)rfp->name;
      }

      return 0;
}

/*
 * This function is a get_value_ method of a vpiHandle, that supports
 * reading bits as a string.
 */
static void signal_get_value(vpiHandle ref, s_vpi_value*vp)
{
      static char buff[1024];
      char*cp;
      unsigned width;
      unsigned val;
      unsigned idx;
      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;

      assert(rfp->bits);
      width = rfp->nbits;
      cp = buff;

      switch (vp->format) {
	  case vpiObjTypeVal:
	  case vpiBinStrVal:
	    for (idx = 0 ;  idx < width ;  idx += 1)
		  switch (rfp->bits[width-idx-1]) {
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
	    for (idx = 0 ;  idx < width ;  idx += 1) {
		  val *= 2;
		  switch (rfp->bits[width-idx-1]) {
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
	    if (width%3) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  unsigned i;
		  for (i = 0 ;  i < width%3 ;  i += 1) {
			v *= 2;
			switch (rfp->bits[width-i-1]) {
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
		  if (x == width%3)
			*cp++ = 'x';
		  else if (x > 0)
			*cp++ = 'X';
		  else if (z == width%3)
			*cp++ = 'z';
		  else if (z > 0)
			*cp++ = 'Z';
		  else
			*cp++ = "01234567"[v];
	    }

	    for (idx = width%3 ;  idx < width ;  idx += 3) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  unsigned i;
		  for (i = idx ;  i < idx+3 ;  i += 1) {
			v *= 2;
			switch (rfp->bits[width-i-1]) {
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
		  unsigned i;
		  for (i = 0 ;  i < width%4 ;  i += 1) {
			v *= 2;
			switch (rfp->bits[width-i-1]) {
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

	    for (idx = width%4 ;  idx < width ;  idx += 4) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  unsigned i;
		  for (i = idx ;  i < idx+4 ;  i += 1) {
			v *= 2;
			switch (rfp->bits[width-i-1]) {
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

static const struct __vpirt vpip_net_rt = {
      vpiNet,
      0,
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
      0,
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

