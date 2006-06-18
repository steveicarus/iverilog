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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpi_const.cc,v 1.36 2006/06/18 04:15:50 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <stdio.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

static int string_get(int code, vpiHandle ref)
{
    struct __vpiStringConst*rfp;

      switch (code) {
          case vpiSize:
	      rfp = (struct __vpiStringConst*)ref;

	      assert((ref->vpi_type->type_code == vpiConstant)
		     || ((ref->vpi_type->type_code == vpiParameter)));

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
      unsigned uint_value;
      struct __vpiStringConst*rfp = (struct __vpiStringConst*)ref;
      int size = strlen(rfp->value);
      char*rbuf = 0;
      char*cp;

      assert((ref->vpi_type->type_code == vpiConstant)
	     || ((ref->vpi_type->type_code == vpiParameter)));

      switch (vp->format) {
	  case vpiObjTypeVal:
	      /* String parameters by default have vpiStringVal values. */
	    vp->format = vpiStringVal;

	  case vpiStringVal:
	    rbuf = need_result_buf(size + 1, RBUF_VAL);
	    strcpy(rbuf, (char*)rfp->value);
	    vp->value.str = rbuf;
	    break;

          case vpiDecStrVal:
	      if (size > 4){
		  // We only support standard integers. Ignore other bytes...
		  size = 4;
		  fprintf(stderr, "Warning (vpi_const.cc): %%d on constant strings only looks "
			  "at first 4 bytes!\n");
	      }
	      rbuf = need_result_buf(size + 1, RBUF_VAL);
	      uint_value = 0;
	      for(int i=0; i<size;i ++){
		  uint_value <<=8;
		  uint_value += (unsigned char)(rfp->value[i]);
	      }
	      sprintf(rbuf, "%u", uint_value);
	      vp->value.str = rbuf;
	      break;

          case vpiBinStrVal:
	      rbuf = need_result_buf(8 * size + 1, RBUF_VAL);
	      cp = rbuf;
	      for(int i=0; i<size;i ++){
		  for(int bit=7;bit>=0; bit--){
		      *cp++ = "01"[ (rfp->value[i]>>bit)&1 ];
		  }
	      }
	      *cp = 0;
	      vp->value.str = rbuf;
	      break;

          case vpiHexStrVal:
	      rbuf = need_result_buf(2 * size + 1, RBUF_VAL);
	      cp = rbuf;
	      for(int i=0; i<size;i++){
		  for(int nibble=1;nibble>=0; nibble--){
		      *cp++ = "0123456789abcdef"[ (rfp->value[i]>>(nibble*4))&15 ];
		  }
	      }
	      *cp = 0;
	      vp->value.str = rbuf;
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

static int free_temp_string(vpiHandle obj)
{
      struct __vpiStringConst*rfp = (struct __vpiStringConst*)obj;
      assert(obj->vpi_type->type_code == vpiConstant);

      free(rfp->value);
      free(rfp);
      return 1;
}

static const struct __vpirt vpip_string_temp_rt = {
      vpiConstant,
      string_get,
      0,
      string_value,
      0,

      0,
      0,
      0,

      free_temp_string
};


vpiHandle vpip_make_string_const(char*text, bool persistent_flag)
{
      struct __vpiStringConst*obj;

      obj = (struct __vpiStringConst*)
	    malloc(sizeof (struct __vpiStringConst));
      obj->base.vpi_type = persistent_flag
	    ? &vpip_string_rt
	    : &vpip_string_temp_rt;
      obj->value = text;

      return &obj->base;
}


struct __vpiStringParam  : public __vpiStringConst {
      const char*basename;
      struct __vpiScope* scope;
};

static char* string_param_get_str(int code, vpiHandle obj)
{
      struct __vpiStringParam*rfp = (struct __vpiStringParam*)obj;
      char *rbuf = need_result_buf(strlen(rfp->basename) + 1, RBUF_STR);

      assert(obj->vpi_type->type_code == vpiParameter);

      switch (code) {
	  case vpiName:
	    strcpy(rbuf, rfp->basename);
	    return rbuf;

	  default:
	    return 0;
      }
}

static vpiHandle string_param_handle(int code, vpiHandle obj)
{
      struct __vpiStringParam*rfp = (struct __vpiStringParam*)obj;

      assert(obj->vpi_type->type_code == vpiParameter);

      switch (code) {
	  case vpiScope:
	    return &rfp->scope->base;

	  default:
	    return 0;
      }
}

static const struct __vpirt vpip_string_param_rt = {
      vpiParameter,
      string_get,
      string_param_get_str,
      string_value,
      0,

      string_param_handle,
      0,
      0,

      0
};


vpiHandle vpip_make_string_param(char*name, char*text)
{
      struct __vpiStringParam*obj;

      obj = (struct __vpiStringParam*)
	    malloc(sizeof (struct __vpiStringParam));
      obj->base.vpi_type = &vpip_string_param_rt;
      obj->value = text;
      obj->basename = name;
      obj->scope = vpip_peek_current_scope();

      return &obj->base;
}

static int binary_get(int code, vpiHandle ref)
{
      struct __vpiBinaryConst*rfp = (struct __vpiBinaryConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant
	     || ref->vpi_type->type_code == vpiParameter);

      switch (code) {
	  case vpiConstType:
	    return vpiBinaryConst;

	  case vpiSigned:
	    return rfp->signed_flag? 1 : 0;

	  case vpiSize:
	    return rfp->bits.size();

	  default:
	    fprintf(stderr, "vvp error: get %d not supported "
		    "by vpiBinaryConst\n", code);
	    assert(0);
	    return 0;
      }
}


static void binary_value(vpiHandle ref, p_vpi_value vp)
{
      assert(ref->vpi_type->type_code == vpiConstant
	     || ref->vpi_type->type_code == vpiParameter);

      struct __vpiBinaryConst*rfp = (struct __vpiBinaryConst*)ref;


      switch (vp->format) {

	  case vpiObjTypeVal:
	  case vpiBinStrVal:
	  case vpiDecStrVal:
	  case vpiOctStrVal:
	  case vpiHexStrVal:
	  case vpiIntVal:
	  case vpiVectorVal:
	  case vpiStringVal:
	  case vpiRealVal:
	    vpip_vec4_get_value(rfp->bits, rfp->bits.size(),
				rfp->signed_flag, vp);
	    break;

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

/*
 * Make a VPI constant from a vector string. The string is normally a
 * ASCII string, with each letter a 4-value bit. The first character
 * may be an 's' if the vector is signed.
 */
vpiHandle vpip_make_binary_const(unsigned wid, char*bits)
{
      struct __vpiBinaryConst*obj;

      obj = new __vpiBinaryConst;
      obj->base.vpi_type = &vpip_binary_rt;

      obj->signed_flag = 0;
      obj->sized_flag = 0;
      obj->bits = vvp_vector4_t(wid);

      const char*bp = bits;
      if (*bp == 's') {
	    bp += 1;
	    obj->signed_flag = 1;
      }

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_bit4_t val = BIT4_0;
	    switch (bp[wid-idx-1]) {
		case '0':
		  val = BIT4_0;
		  break;
		case '1':
		  val = BIT4_1;
		  break;
		case 'x':
		  val = BIT4_X;
		  break;
		case 'z':
		  val = BIT4_Z;
		  break;
	    }

	    obj->bits.set_bit(idx, val);
      }

      free(bits);
      return &(obj->base);
}

struct __vpiBinaryParam  : public __vpiBinaryConst {
      const char*basename;
      struct __vpiScope*scope;
};

static char* binary_param_get_str(int code, vpiHandle obj)
{
      struct __vpiBinaryParam*rfp = (struct __vpiBinaryParam*)obj;
      char *rbuf = need_result_buf(strlen(rfp->basename) + 1, RBUF_STR);

      assert(obj->vpi_type->type_code == vpiParameter);

      switch (code) {
	  case vpiName:
	    strcpy(rbuf, rfp->basename);
	    return rbuf;

	  default:
	    return 0;
      }
}

static vpiHandle binary_param_handle(int code, vpiHandle obj)
{
      struct __vpiBinaryParam*rfp = (struct __vpiBinaryParam*)obj;

      assert(obj->vpi_type->type_code == vpiParameter);

      switch (code) {
	  case vpiScope:
	    return &rfp->scope->base;

	  default:
	    return 0;
      }
}

static const struct __vpirt vpip_binary_param_rt = {
      vpiParameter,
      binary_get,
      binary_param_get_str,
      binary_value,
      0,

      binary_param_handle,
      0,
      0,

      0
};

vpiHandle vpip_make_binary_param(char*name, const vvp_vector4_t&bits,
				 bool signed_flag)
{
      struct __vpiBinaryParam*obj = new __vpiBinaryParam;

      obj->base.vpi_type = &vpip_binary_param_rt;
      obj->bits = bits;
      obj->signed_flag = signed_flag? 1 : 0;
      obj->sized_flag = 0;
      obj->basename = name;
      obj->scope = vpip_peek_current_scope();

      return &obj->base;
}


static int dec_get(int code, vpiHandle ref)
{

      switch (code) {
	  case vpiConstType:
	    return vpiDecConst;

	  case vpiSigned:
	    return 1;

	  case vpiSize:
	    return 32;

	  default:
	    fprintf(stderr, "vvp error: get %d not supported "
		    "by vpiDecConst\n", code);
	    assert(0);
	    return 0;
      }
}


static void dec_value(vpiHandle ref, p_vpi_value vp)
{
      struct __vpiDecConst*rfp = (struct __vpiDecConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant);
      char*rbuf = need_result_buf(64 + 1, RBUF_VAL);
      char*cp = rbuf;

      switch (vp->format) {

	  case vpiObjTypeVal:
	  case vpiIntVal: {
		vp->value.integer = rfp->value;
		break;
	  }

          case vpiDecStrVal:
	      sprintf(rbuf, "%d", rfp->value);

	      vp->value.str = rbuf;
	      break;

          case vpiBinStrVal:
	      for(int bit=31; bit<=0;bit--){
		  *cp++ = "01"[ (rfp->value>>bit)&1 ];
	      }
	      *cp = 0;

	      vp->value.str = rbuf;
	      break;

          case vpiHexStrVal:
	      sprintf(rbuf, "%08x", rfp->value);

	      vp->value.str = rbuf;
	      break;

          case vpiOctStrVal:
	      sprintf(rbuf, "%011x", rfp->value);

	      vp->value.str = rbuf;
	      break;

	  default:
	    fprintf(stderr, "vvp error (vpi_const.cc): format %d not supported "
		    "by vpiDecConst\n", vp->format);
	    vp->format = vpiSuppressVal;
	    break;
      }
}

static const struct __vpirt vpip_dec_rt = {
      vpiConstant,
      dec_get,
      0,
      dec_value,
      0,
      0,
      0
};

vpiHandle vpip_make_dec_const(struct __vpiDecConst*obj, int value)
{
      obj->base.vpi_type = &vpip_dec_rt;
      obj->value = value;

      return &(obj->base);
}

vpiHandle vpip_make_dec_const(int value)
{
      struct __vpiDecConst*obj;

      obj = (struct __vpiDecConst*)
	    malloc(sizeof (struct __vpiDecConst));
      return vpip_make_dec_const(obj, value);
}


static int real_get(int code, vpiHandle ref)
{

      switch (code) {
	  case vpiConstType:
	    return vpiRealConst;

	  case vpiSigned:
	    return 1;

	  default:
	    fprintf(stderr, "vvp error: get %d not supported "
		    "by vpiDecConst\n", code);
	    assert(0);
	    return 0;
      }
}

static void real_value(vpiHandle ref, p_vpi_value vp)
{
      struct __vpiRealConst*rfp = (struct __vpiRealConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant);

      switch (vp->format) {
	  case vpiObjTypeVal:
	    vp->format = vpiRealVal;
	  case vpiRealVal:
	    vp->value.real = rfp->value;
	    break;
	  default:
	    assert(0);
      }
}

static const struct __vpirt vpip_real_rt = {
      vpiConstant,
      real_get,
      0,
      real_value,
      0,
      0,
      0
};

vpiHandle vpip_make_real_const(struct __vpiRealConst*obj, double value)
{
      obj->base.vpi_type = &vpip_real_rt;
      obj->value = value;
      return &(obj->base);
}

vpiHandle vpip_make_real_const(double value)
{
      struct __vpiRealConst*obj;
      obj =(struct __vpiRealConst*) malloc(sizeof (struct __vpiRealConst));
      return vpip_make_real_const(obj, value);
}

/*
 * $Log: vpi_const.cc,v $
 * Revision 1.36  2006/06/18 04:15:50  steve
 *  Add support for system functions in continuous assignments.
 *
 * Revision 1.35  2006/03/18 22:51:10  steve
 *  Syntax for carrying sign with parameter.
 *
 * Revision 1.34  2006/03/08 05:29:42  steve
 *  Add support for logic parameters.
 *
 * Revision 1.33  2006/03/06 05:43:15  steve
 *  Cleanup vpi_const to use vec4 values.
 *
 * Revision 1.32  2004/10/04 01:10:59  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.31  2004/05/18 18:43:38  steve
 *  Allow vpiParamter as a string type.
 */

