/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_const.c,v 1.3 2002/08/12 01:35:05 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <assert.h>
# include  <string.h>
# include  <stdio.h>

static unsigned vpip_bits_to_dec_str(const vpip_bit_t*bits, unsigned nbits,
				     char*buf, unsigned nbuf, int signed_flag)
{
      unsigned idx, len;
      unsigned count_x = 0, count_z = 0;
      unsigned long val = 0;

      assert( nbits <= 8*sizeof(val) );

      for (idx = 0 ;  idx < nbits ;  idx += 1) {
	    val *= 2;
	    if (B_ISZ(bits[nbits-idx-1]))
		  count_z += 1;
	    else if (B_ISX(bits[nbits-idx-1]))
		  count_x += 1;
	    else if (B_IS1(bits[nbits-idx-1]))
		  val += 1;
      }

      if (count_x == nbits) {
	    len = 1;
	    buf[0] = 'x';
	    buf[1] = 0;
      } else if (count_x > 0) {
	    len = 1;
	    buf[0] = 'X';
	    buf[1] = 0;
      } else if (count_z == nbits) {
	    len = 1;
	    buf[0] = 'z';
	    buf[1] = 0;
      } else if (count_z > 0) {
	    len = 1;
	    buf[0] = 'Z';
	    buf[1] = 0;
      } else {
	    if (signed_flag && B_IS1(bits[nbits-1])) {
		  long tmp = -1;
		  assert(sizeof(tmp) == sizeof(val));
		  tmp <<= nbits;
		  tmp |= val;
		  sprintf(buf, "%ld", tmp);
		  len = strlen(buf);
	    } else {
		  sprintf(buf, "%lu", val);
		  len = strlen(buf);
	    }
      }
      return len;
}

/*
 * This function is used in a couple places to interpret a bit string
 * as a value.
 */
void vpip_bits_get_value(const vpip_bit_t*bits, unsigned nbits,
			 s_vpi_value*vp, int signed_flag)
{
      static char buff[1024];
      static s_vpi_vecval vect[64];
      char* cp = buff;
      unsigned val;
      unsigned idx;
      int isx;

      vp->value.str = buff;

      switch (vp->format) {
	  case vpiObjTypeVal:
	  case vpiBinStrVal:
	    for (idx = 0 ;  idx < nbits ;  idx += 1) {
		  if (B_IS0(bits[nbits-idx-1]))
			*cp++ = '0';
		  else if (B_IS1(bits[nbits-idx-1]))
			*cp++ = '1';
		  else if (B_ISZ(bits[nbits-idx-1]))
			*cp++ = 'z';
		  else
			*cp++ = 'x';
		  }
	    vp->format = vpiBinStrVal;
	    *cp++ = 0;
	    break;

	  case vpiDecStrVal:
	    cp += vpip_bits_to_dec_str(bits, nbits, cp,
				       1024-(cp-buff), signed_flag);
	    break;

	  case vpiOctStrVal:
	    if (nbits%3) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  unsigned i;
		  for (i = 0 ;  i < nbits%3 ;  i += 1) {
			v *= 2;
			if (B_IS0(bits[nbits-i-1]))
			      ;
			else if (B_IS1(bits[nbits-i-1]))
			      v += 1;
			else if (B_ISX(bits[nbits-i-1]))
			      x += 1;
			else if (B_ISZ(bits[nbits-i-1]))
			      z += 1;
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
			if (B_IS0(bits[nbits-i-1]))
			      ;
			else if (B_IS1(bits[nbits-i-1]))
			      v += 1;
			else if (B_ISX(bits[nbits-i-1]))
			      x += 1;
			else if (B_ISZ(bits[nbits-i-1]))
			      z += 1;

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
	    *cp++ = 0;
	    break;

	  case vpiHexStrVal:
	    if (nbits%4) {
		  unsigned x = 0;
		  unsigned z = 0;
		  unsigned v = 0;
		  unsigned i;
		  for (i = 0 ;  i < nbits%4 ;  i += 1) {
			v *= 2;
			if (B_IS0(bits[nbits-i-1]))
			      ;
			else if (B_IS1(bits[nbits-i-1]))
			      v += 1;
			else if (B_ISX(bits[nbits-i-1]))
			      x += 1;
			else if (B_ISZ(bits[nbits-i-1]))
			      z += 1;

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
			if (B_IS0(bits[nbits-i-1]))
			      ;
			else if (B_IS1(bits[nbits-i-1]))
			      v += 1;
			else if (B_ISX(bits[nbits-i-1]))
			      x += 1;
			else if (B_ISZ(bits[nbits-i-1]))
			      z += 1;


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
	    *cp++ = 0;
	    break;

	  case vpiIntVal:
	    val = 0;
	    isx = 0;
	    for (idx = 0 ;  idx < nbits ;  idx += 1) {
		  val *= 2;
		  if (B_ISXZ(bits[nbits-idx-1]))
			  isx = 1;
		  else if (B_IS1(bits[nbits-idx-1]))
			  val += 1;
	    }
	    if(isx)
		    vp->value.integer = 0;
	    else
		    vp->value.integer = val;
	    break;

	  case vpiStringVal:
	      /* Turn the bits into an ascii string, terminated by a
		 null. This is actually a bit tricky as nulls in the
		 bit array would terminate the C string. I therefore
		 translate them to ascii ' ' characters. */
	    assert(nbits%8 == 0);
	    for (idx = nbits ;  idx >= 8 ;  idx -= 8) {
		  char tmp = 0;
		  unsigned bdx;
		  for (bdx = 8 ;  bdx > 0 ;  bdx -= 1) {
			tmp <<= 1;
			if (B_IS1(bits[idx-8+bdx-1]))
			      tmp |= 1;
		  }
		  *cp++ = tmp? tmp : ' ';
	    }
	    *cp++ = 0;
	    break;

	  case vpiVectorVal:
	    vp->value.vector = vect;
	    for (idx = 0 ;  idx < nbits ;  idx += 1) {
		  int major = idx/32;
		  int minor = idx%32;

		  vect[major].aval &= (1<<minor) - 1;
		  vect[major].bval &= (1<<minor) - 1;

		  if (B_IS1(bits[idx]) || B_ISX(bits[idx]))
			vect[major].aval |= 1<<minor;
		  if (B_ISXZ(bits[idx]))
			vect[major].bval |= 1<<minor;
	    }
	    break;

	  default:
	    *cp++ = '(';
	    *cp++ = '?';
	    *cp++ = ')';
	    *cp++ = 0;
	    vp->format = vpiStringVal;
	    break;
      }
}

void vpip_bits_set_value(vpip_bit_t*bits, unsigned nbits, s_vpi_value*vp)
{
      switch (vp->format) {

	  case vpiScalarVal:
	    switch (vp->value.scalar) {
		case vpi0:
		  bits[0] = St0;
		  break;
		case vpi1:
		  bits[0] = St1;
		  break;
		case vpiX:
		  bits[0] = StX;
		  break;
		case vpiZ:
		  bits[0] = HiZ;
		  break;
		default:
		  assert(0);
	    }
	    break;

	  case vpiVectorVal: {
		unsigned long aval = vp->value.vector->aval;
		unsigned long bval = vp->value.vector->bval;
		int idx;
		for (idx = 0 ;  idx < nbits ;  idx += 1) {
		      int bit = (aval&1) | ((bval<<1)&2);
		      switch (bit) {
			  case 0:
			    bits[idx] = St0;
			    break;
			  case 1:
			    bits[idx] = St1;
			    break;
			  case 2:
			    bits[idx] = HiZ;
			    break;
			  case 3:
			    bits[idx] = StX;
			    break;
		      }
		      aval >>= 1;
		      bval >>= 1;
		}
		break;
	  }

	  case vpiIntVal: {
		long val = vp->value.integer;
		unsigned idx;

		for (idx = 0 ;  idx < nbits ;  idx += 1) {
		      bits[idx] = (val&1)? St1 : St0;
		      val >>= 1;
		}
		break;
	  }

	  default:
	    assert(0);
      }
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
	    vp->value.str = (char*)rfp->value;
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
      vpip_bits_get_value(rfp->bits, rfp->nbits, vp, 0);
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
				 const vpip_bit_t*bits,
				 unsigned nbits)
{
      ref->base.vpi_type = &vpip_number_rt;
      ref->bits = bits;
      ref->nbits = nbits;
      return &(ref->base);
}

/*
 * $Log: vpi_const.c,v $
 * Revision 1.3  2002/08/12 01:35:05  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/04/24 15:47:37  steve
 *  Fix setting StX in vpip_bits_set_value.
 *
 * Revision 1.1  2001/03/14 19:27:44  steve
 *  Rearrange VPI support libraries.
 *
 * Revision 1.17  2001/01/07 18:22:15  steve
 *  Assert on length of bit vector.
 *
 * Revision 1.16  2001/01/06 22:22:17  steve
 *  Support signed decimal display of variables.
 *
 * Revision 1.15  2000/12/10 19:15:19  steve
 *  vpiStringVal handles leding nulls as blanks. (PR#62)
 *
 * Revision 1.14  2000/12/02 02:40:56  steve
 *  Support for %s in $display (PR#62)
 *
 * Revision 1.13  2000/09/23 16:34:47  steve
 *  Handle unknowns in decimal strings.
 *
 * Revision 1.12  2000/08/20 17:49:05  steve
 *  Clean up warnings and portability issues.
 *
 * Revision 1.11  2000/08/08 01:47:40  steve
 *  Add vpi_vlog_info support from Adrian
 *
 * Revision 1.10  2000/07/08 22:40:07  steve
 *  Allow set vpiIntVal on bitset type objects.
 *
 * Revision 1.9  2000/05/18 03:27:32  steve
 *  Support writing scalars and vectors to signals.
 *
 * Revision 1.8  2000/05/07 18:20:08  steve
 *  Import MCD support from Stephen Tell, and add
 *  system function parameter support to the IVL core.
 *
 * Revision 1.7  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.6  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
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

