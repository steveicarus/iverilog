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
#ident "$Id: vpi_const.c,v 1.5 1999/12/15 04:01:14 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <assert.h>
# include  <string.h>
# include  <stdio.h>


/*
 * This function is used in a couple places to interpret a bit string
 * as a value.
 */
void vpip_bits_get_value(enum vpip_bit_t*bits, unsigned nbits, s_vpi_value*vp)
{
      static char buff[1024];
      char*cp;
      unsigned val;
      unsigned idx;

      cp = buff;

      switch (vp->format) {
	  case vpiObjTypeVal:
	  case vpiBinStrVal:
	    for (idx = 0 ;  idx < nbits ;  idx += 1)
		  switch (bits[nbits-idx-1]) {
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
	    for (idx = 0 ;  idx < nbits ;  idx += 1) {
		  val *= 2;
		  switch (bits[nbits-idx-1]) {
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
	    if (nbits%3) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  unsigned i;
		  for (i = 0 ;  i < nbits%3 ;  i += 1) {
			v *= 2;
			switch (bits[nbits-i-1]) {
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
		  if (x == nbits%3)
			*cp++ = 'x';
		  else if (x > 0)
			*cp++ = 'X';
		  else if (z == nbits%3)
			*cp++ = 'z';
		  else if (z > 0)
			*cp++ = 'Z';
		  else
			*cp++ = "01234567"[v];
	    }

	    for (idx = nbits%3 ;  idx < nbits ;  idx += 3) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  unsigned i;
		  for (i = idx ;  i < idx+3 ;  i += 1) {
			v *= 2;
			switch (bits[nbits-i-1]) {
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
	    if (nbits%4) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  unsigned i;
		  for (i = 0 ;  i < nbits%4 ;  i += 1) {
			v *= 2;
			switch (bits[nbits-i-1]) {
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
		  if (x == nbits%4)
			*cp++ = 'x';
		  else if (x > 0)
			*cp++ = 'X';
		  else if (z == nbits%4)
			*cp++ = 'z';
		  else if (z > 0)
			*cp++ = 'Z';
		  else
			*cp++ = "0123456789abcdef"[v];
	    }

	    for (idx = nbits%4 ;  idx < nbits ;  idx += 4) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  unsigned i;
		  for (i = idx ;  i < idx+4 ;  i += 1) {
			v *= 2;
			switch (bits[nbits-i-1]) {
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

static int string_get(int code, vpiHandle ref)
{
      struct __vpiStringConst*rfp = (struct __vpiStringConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant);

      switch (code) {
	  case vpiConstType:
	    return vpiStringConst;

	  default:
	    assert(0);
	    return 0;
      }
}

static void string_value(vpiHandle ref, p_vpi_value vp)
{
      struct __vpiStringConst*rfp = (struct __vpiStringConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant);

      switch (vp->format) {
	  case vpiObjTypeVal:
	  case vpiStringVal:
	    vp->value.str = rfp->value;
	    vp->format = vpiStringVal;
	    break;

	  default:
	    vp->format = vpiSuppressVal;
	    break;
      }
}

static int number_get(int code, vpiHandle ref)
{
      struct __vpiNumberConst*rfp = (struct __vpiNumberConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant);

      switch (code) {
	  case vpiConstType:
	    return vpiBinaryConst;

	  default:
	    assert(0);
	    return 0;
      }
}

static void number_value(vpiHandle ref, p_vpi_value vp)
{
      struct __vpiNumberConst*rfp = (struct __vpiNumberConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant);
      vpip_bits_get_value(rfp->bits, rfp->nbits, vp);
}

static const struct __vpirt vpip_string_rt = {
      vpiConstant,
      string_get,
      0,
      string_value,
      0,
      0,
      0
};

static const struct __vpirt vpip_number_rt = {
      vpiConstant,
      number_get,
      0,
      number_value,
      0,
      0,
      0
};

vpiHandle vpip_make_string_const(struct __vpiStringConst*ref, const char*val)
{
      ref->base.vpi_type = &vpip_string_rt;
      ref->value = val;
      return &(ref->base);
}

vpiHandle vpip_make_number_const(struct __vpiNumberConst*ref,
				 const enum vpip_bit_t*bits,
				 unsigned nbits)
{
      ref->base.vpi_type = &vpip_number_rt;
      ref->bits = bits;
      ref->nbits = nbits;
      return &(ref->base);
}

/*
 * $Log: vpi_const.c,v $
 * Revision 1.5  1999/12/15 04:01:14  steve
 *  Add the VPI implementation of $readmemh.
 *
 * Revision 1.4  1999/11/06 22:16:50  steve
 *  Get the $strobe task working.
 *
 * Revision 1.3  1999/11/06 16:52:16  steve
 *  complete value retrieval for number constants.
 *
 * Revision 1.2  1999/11/06 16:00:18  steve
 *  Put number constants into a static table.
 *
 * Revision 1.1  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 */

