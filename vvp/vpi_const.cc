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
#ident "$Id: vpi_const.cc,v 1.9 2002/01/25 03:24:19 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <stdio.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

static char buf[4096];

static int string_get(int code, vpiHandle ref)
{
    struct __vpiStringConst*rfp;

      switch (code) {
          case vpiSize:
	      rfp = (struct __vpiStringConst*)ref;
	      assert(ref->vpi_type->type_code == vpiConstant);

	      //fprintf(stderr, "String:|%s|, Length: %d\n", rfp->value, strlen(rfp->value));
	      return strlen(rfp->value)*8;

          case vpiSigned:
	      return 0;

	  case vpiConstType:
	      return vpiStringConst;

	  default:
	      fprintf(stderr, "vvp error: get %d not supported "
		      "by vpiStringConst\n", code);
	      assert(0);
	      return 0;
      }
}

static void string_value(vpiHandle ref, p_vpi_value vp)
{
      int size;
      unsigned uint_value;
      char *cp;

      struct __vpiStringConst*rfp = (struct __vpiStringConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant);

      switch (vp->format) {
	  case vpiObjTypeVal:
	  case vpiStringVal:
	    vp->value.str = (char*)rfp->value;
	    vp->format = vpiStringVal;
	    break;

          case vpiDecStrVal:
	      size = strlen(rfp->value);
	      if (size > 4){
		  // We only support standard integers. Ignore other bytes...
		  size = 4;	
		  fprintf(stderr, "Warning (vpi_const.cc): %%d on constant strings only looks "
			  "at first 4 bytes!\n");
	      }

	      uint_value = 0;
	      for(int i=0; i<size;i ++){
		  uint_value <<=8;
		  uint_value += (unsigned char)(rfp->value[i]);
	      }

	      sprintf(buf, "%u", uint_value);

	      vp->format = vpiDecStrVal;
	      vp->value.str = buf;
	      break;

          case vpiBinStrVal:
	      size = strlen(rfp->value);
	      if (size*8 > (int)(sizeof(buf)/sizeof(char))-1 ){
		  // Avoid overflow of 'buf'
		  // 4096 should be sufficient for most cases though. ;-)
		  size = (sizeof(buf)/sizeof(char)-2)/8;	
	      }

	      cp = buf;
	      for(int i=0; i<size;i ++){
		  for(int bit=7;bit>=0; bit--){
		      *cp++ = "01"[ (rfp->value[i]>>bit)&1 ];
		  }
	      }
	      *cp = 0;

	      vp->format = vpiBinStrVal;
	      vp->value.str = buf;
	      break;

          case vpiHexStrVal:
	      size = strlen(rfp->value);
	      if (size*2 > (int)(sizeof(buf)/sizeof(char))-1 ){
		  // Avoid overflow of 'buf'
		  // 4096 should be sufficient for most cases though. ;-)
		  size = (sizeof(buf)/sizeof(char)-2)/2;	
	      }

	      cp = buf;
	      for(int i=0; i<size;i++){
		  for(int nibble=1;nibble>=0; nibble--){
		      *cp++ = "0123456789abcdef"[ (rfp->value[i]>>(nibble*4))&15 ];
		  }
	      }
	      *cp = 0;

	      vp->format = vpiHexStrVal;
	      vp->value.str = buf;
	      break;

          case vpiOctStrVal:
	      fprintf(stderr, "ERROR (vpi_const.cc): %%o display of constant strings not yet implemented\n");
	      assert(0);
	      break;

	  default:
	    fprintf(stderr, "ERROR (vpi_const.cc): vp->format: %d\n", vp->format);
	    assert(0);

	    vp->format = vpiSuppressVal;
	    break;
      }
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


vpiHandle vpip_make_string_const(char*text)
{
      struct __vpiStringConst*obj;

      obj = (struct __vpiStringConst*)
	    malloc(sizeof (struct __vpiStringConst));
      obj->base.vpi_type = &vpip_string_rt;
      obj->value = text;

      return &obj->base;
}


static int binary_get(int code, vpiHandle ref)
{
      struct __vpiBinaryConst*rfp = (struct __vpiBinaryConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant);

      switch (code) {
	  case vpiConstType:
	    return vpiBinaryConst;

	  case vpiSigned: // FIXME: Need to get signed flag right.
	    return 0;

	  case vpiSize:
	    return rfp->nbits;

	  default:
	    fprintf(stderr, "vvp error: get %d not supported "
		    "by vpiBinaryConst\n", code);
	    assert(0);
	    return 0;
      }
}


static void binary_value(vpiHandle ref, p_vpi_value vp)
{
      struct __vpiBinaryConst*rfp = (struct __vpiBinaryConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant);

      switch (vp->format) {

	  case vpiObjTypeVal:
	  case vpiBinStrVal:
	    assert(rfp->nbits < sizeof buf);
	    for (unsigned idx = 0 ;  idx < rfp->nbits ;  idx += 1) {
		  unsigned nibble = idx/4;
		  unsigned shift  = 2 * (idx%4);
		  unsigned val = (rfp->bits[nibble] >> shift) & 3;

		  buf[rfp->nbits-idx-1] = "01xz"[val];
	    }
	    buf[rfp->nbits] = 0;
	    vp->value.str = buf;
	    vp->format = vpiBinStrVal;
	    break;

	  case vpiDecStrVal: {
		unsigned long val = 0;
		unsigned count_x = 0, count_z = 0;

		vp->value.str = buf;

		for (unsigned idx = 0 ;  idx < rfp->nbits ;  idx += 1) {
		      unsigned nibble = idx/4;
		      unsigned shift  = 2 * (idx%4);
		      unsigned bit_val = (rfp->bits[nibble] >> shift) & 3;
		      switch (bit_val) {
			  case 0:
			    break;
			  case 1:
			    val |= 1 << idx;
			    break;
			  case 2:
			    count_x += 1;
			    break;
			  case 3:
			    count_z += 1;
			    break;
		      }
		}

		if (count_z == rfp->nbits) {
		      sprintf(buf, "z");

		} else if (count_x == rfp->nbits) {
		      sprintf(buf, "x");

		} else if ((count_z > 0) && (count_x == 0)) {
		      sprintf(buf, "Z");

		} else if (count_x > 0) {
		      sprintf(buf, "X");

		} else {
		      sprintf(buf, "%lu", val);
		}
		break;
	  }

	  case vpiIntVal: {
		unsigned val = 0;

		for (unsigned idx = 0 ;  idx < rfp->nbits ;  idx += 1) {
		      unsigned nibble = idx/4;
		      unsigned shift  = 2 * (idx%4);
		      unsigned bit_val = (rfp->bits[nibble] >> shift) & 3;
		      if (bit_val > 1) {
			    vp->value.integer = 0;
			    return;
		      } else {
			    val |= bit_val << idx;
		      }
		}
		vp->value.integer = val;
		break;
	  }

	  default:
	    fprintf(stderr, "vvp error: format %d not supported "
		    "by vpiBinaryConst\n", vp->format);
	    vp->format = vpiSuppressVal;
	    break;
      }
}

static const struct __vpirt vpip_binary_rt = {
      vpiConstant,
      binary_get,
      0,
      binary_value,
      0,
      0,
      0
};

vpiHandle vpip_make_binary_const(unsigned wid, char*bits)
{
      struct __vpiBinaryConst*obj;

      obj = (struct __vpiBinaryConst*)
	    malloc(sizeof (struct __vpiBinaryConst));
      obj->base.vpi_type = &vpip_binary_rt;

      obj->nbits = wid;
      obj->bits = (unsigned char*)malloc((obj->nbits + 3) / 4);
      memset(obj->bits, 0, (obj->nbits + 3) / 4);

      for (unsigned idx = 0 ;  idx < obj->nbits ;  idx += 1) {
	    unsigned nibble = idx / 4;
	    unsigned val = 0;
	    switch (bits[wid-idx-1]) {
		case '0':
		  val = 0;
		  break;
		case '1':
		  val = 1;
		  break;
		case 'x':
		  val = 2;
		  break;
		case 'z':
		  val = 3;
		  break;
	    }

	    obj->bits[nibble] |= val << (2 * (idx%4));
      }

      free(bits);
      return &(obj->base);
}


/*
 * $Log: vpi_const.cc,v $
 * Revision 1.9  2002/01/25 03:24:19  steve
 *  Support display of strings with umber formats. (Tom Verbeure)
 *
 * Revision 1.8  2002/01/15 03:21:18  steve
 *  Support DesSTrVal for binary constants.
 *
 * Revision 1.7  2001/09/15 18:27:05  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.6  2001/08/08 00:57:20  steve
 *  Unused variable warnings.
 *
 * Revision 1.5  2001/07/11 04:40:52  steve
 *  Get endian of vpiIntVal from constants.
 *
 * Revision 1.4  2001/04/04 05:07:19  steve
 *  Get intval from a binary constant.
 *
 * Revision 1.3  2001/04/04 04:33:08  steve
 *  Take vector form as parameters to vpi_call.
 *
 * Revision 1.2  2001/04/02 00:24:31  steve
 *  Take numbers as system task parameters.
 *
 * Revision 1.1  2001/03/18 04:35:18  steve
 *  Add support for string constants to VPI.
 *
 */

