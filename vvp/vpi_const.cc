/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

# include  "vpi_priv.h"
# include  "compile.h"
# include  "config.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <cstdio>
# include  <cstdlib>
# include  <cstring>
# include  <cassert>

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

          case vpiAutomatic:
	      return 0;

#ifdef CHECK_WITH_VALGRIND
          case _vpiFromThr:
	      return _vpiNoThr;
#endif

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
      p_vpi_vecval vecp;
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

          case vpiIntVal:
	      vp->value.integer = 0;
	      for(int i=0; i<size;i ++){
		  for(int bit=7;bit>=0; bit--){
		      vp->value.integer <<= 1;
		      vp->value.integer += (rfp->value[i]>>bit)&1;
		  }
	      }
	      break;

          case vpiVectorVal:
              vp->value.vector = (p_vpi_vecval)
                                 need_result_buf((size+3)/4*
                                                  sizeof(s_vpi_vecval),
                                                 RBUF_VAL);
              uint_value = 0;
              vecp = vp->value.vector;
              vecp->aval = vecp->bval = 0;
	      for(int i=0; i<size;i ++){
		  vecp->aval |= rfp->value[i] << uint_value*8;
		  uint_value += 1;
		  if (uint_value > 3) {
		      uint_value = 0;
		      vecp += 1;
		      vecp->aval = vecp->bval = 0;
		  }
	      }
	      break;


	  default:
	    fprintf(stderr, "ERROR (vpi_const.cc): vp->format: %d\n",
	            (int)vp->format);
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

      delete [] rfp->value;
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

/*
 * Strings are described at the level of the vvp source as a string
 * with literal characters or octal escapes. No other escapes are
 * included, they are processed already by the compiler that generated
 * the vvp source.
 */
static void vpip_process_string(struct __vpiStringConst*obj)
{
      char*chr = obj->value;
      char*dp = obj->value;

      while (*chr) {
	    char next_char = *chr;

	      /* Process octal escapes that I might find. */
	    if (*chr == '\\') {
		  for (int idx = 1 ;  idx <= 3 ;  idx += 1) {
			assert(chr[idx] != 0);
			assert(chr[idx] < '8');
			assert(chr[idx] >= '0');
			next_char = next_char*8 + chr[idx] - '0';
		  }
		  chr += 3;
	    }
	    *dp++ = next_char;
	    chr += 1;
      }
      *dp = 0;
      obj->value_len = dp - obj->value;
}

vpiHandle vpip_make_string_const(char*text, bool persistent_flag)
{
      struct __vpiStringConst*obj;

      obj = (struct __vpiStringConst*)
	    malloc(sizeof (struct __vpiStringConst));
      obj->base.vpi_type = persistent_flag
	    ? &vpip_string_rt
	    : &vpip_string_temp_rt;
      obj->value = text;
      obj->value_len = 0;
      vpip_process_string(obj);

      return &obj->base;
}


struct __vpiStringParam  : public __vpiStringConst {
      const char*basename;
      struct __vpiScope* scope;
      unsigned file_idx;
      unsigned lineno;
};

static int string_param_get(int code, vpiHandle ref)
{
      struct __vpiStringParam*rfp = (struct __vpiStringParam*)ref;

      assert(ref->vpi_type->type_code == vpiParameter);

      if (code == vpiLineNo) {
	    return rfp->lineno;
      }

      return string_get(code, ref);
}

static char* string_param_get_str(int code, vpiHandle obj)
{
      struct __vpiStringParam*rfp = (struct __vpiStringParam*)obj;

      assert(obj->vpi_type->type_code == vpiParameter);

      if (code == vpiFile) {
	    return simple_set_rbuf_str(file_names[rfp->file_idx]);
      }

      return generic_get_str(code, &rfp->scope->base, rfp->basename, NULL);
}

static vpiHandle string_param_handle(int code, vpiHandle obj)
{
      struct __vpiStringParam*rfp = (struct __vpiStringParam*)obj;

      assert(obj->vpi_type->type_code == vpiParameter);

      switch (code) {
	  case vpiScope:
	    return &rfp->scope->base;

	  case vpiModule:
	    return vpip_module(rfp->scope);

	  default:
	    return 0;
      }
}

static const struct __vpirt vpip_string_param_rt = {
      vpiParameter,
      string_param_get,
      string_param_get_str,
      string_value,
      0,

      string_param_handle,
      0,
      0,

      0
};


vpiHandle vpip_make_string_param(char*name, char*text,
                                 long file_idx, long lineno)
{
      struct __vpiStringParam*obj;

      obj = (struct __vpiStringParam*)
	    malloc(sizeof (struct __vpiStringParam));
      obj->base.vpi_type = &vpip_string_param_rt;
      obj->value = text;
      obj->value_len = 0;
      obj->basename = name;
      obj->scope = vpip_peek_current_scope();
      obj->file_idx = (unsigned) file_idx;
      obj->lineno = (unsigned) lineno;

      vpip_process_string(obj);

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

	  case vpiLineNo:
	    return 0;  // Not implemented for now!

	  case vpiSigned:
	    return rfp->signed_flag? 1 : 0;

	  case vpiSize:
	    return rfp->bits.size();

          case vpiAutomatic:
	    return 0;

#ifdef CHECK_WITH_VALGRIND
          case _vpiFromThr:
	      return _vpiNoThr;
#endif

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
          case vpiScalarVal:
	  case vpiIntVal:
	  case vpiVectorVal:
	  case vpiStringVal:
	  case vpiRealVal:
	    vpip_vec4_get_value(rfp->bits, rfp->bits.size(),
				rfp->signed_flag, vp);
	    break;

	  default:
	    fprintf(stderr, "vvp error: format %d not supported "
		    "by vpiBinaryConst\n", (int)vp->format);
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
vpiHandle vpip_make_binary_const(unsigned wid, const char*bits)
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

      return &(obj->base);
}

struct __vpiBinaryParam  : public __vpiBinaryConst {
      const char*basename;
      struct __vpiScope*scope;
      unsigned file_idx;
      unsigned lineno;
};

static int binary_param_get(int code, vpiHandle ref)
{
      struct __vpiBinaryParam*rfp = (struct __vpiBinaryParam*)ref;

      assert(ref->vpi_type->type_code == vpiParameter);

      if (code == vpiLineNo) {
	    return rfp->lineno;
      }

      return binary_get(code, ref);
}

static char* binary_param_get_str(int code, vpiHandle obj)
{
      struct __vpiBinaryParam*rfp = (struct __vpiBinaryParam*)obj;

      assert(obj->vpi_type->type_code == vpiParameter);

      if (code == vpiFile) {
	    return simple_set_rbuf_str(file_names[rfp->file_idx]);
      }

      return generic_get_str(code, &rfp->scope->base, rfp->basename, NULL);
}

static vpiHandle binary_param_handle(int code, vpiHandle obj)
{
      struct __vpiBinaryParam*rfp = (struct __vpiBinaryParam*)obj;

      assert(obj->vpi_type->type_code == vpiParameter);

      switch (code) {
	  case vpiScope:
	    return &rfp->scope->base;

	  case vpiModule:
	    return vpip_module(rfp->scope);

	  default:
	    return 0;
      }
}

static const struct __vpirt vpip_binary_param_rt = {
      vpiParameter,
      binary_param_get,
      binary_param_get_str,
      binary_value,
      0,

      binary_param_handle,
      0,
      0,

      0
};

vpiHandle vpip_make_binary_param(char*name, const vvp_vector4_t&bits,
				 bool signed_flag,
				 long file_idx, long lineno)
{
      struct __vpiBinaryParam*obj = new __vpiBinaryParam;

      obj->base.vpi_type = &vpip_binary_param_rt;
      obj->bits = bits;
      obj->signed_flag = signed_flag? 1 : 0;
      obj->sized_flag = 0;
      obj->basename = name;
      obj->scope = vpip_peek_current_scope();
      obj->file_idx = (unsigned) file_idx;
      obj->lineno = (unsigned) lineno;

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

          case vpiAutomatic:
	    return 0;

#ifdef CHECK_WITH_VALGRIND
          case _vpiFromThr:
	      return _vpiNoThr;
#endif

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
		    "by vpiDecConst\n", (int)vp->format);
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
	  case vpiLineNo:
	    return 0;  // Not implemented for now!

	  case vpiSize:
	    return 1;

	  case vpiConstType:
	    return vpiRealConst;

	  case vpiSigned:
	    return 1;

          case vpiAutomatic:
	    return 0;

#ifdef CHECK_WITH_VALGRIND
          case _vpiFromThr:
	      return _vpiNoThr;
#endif

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
      assert((ref->vpi_type->type_code == vpiConstant) ||
             (ref->vpi_type->type_code == vpiParameter));

       vpip_real_get_value(rfp->value, vp);
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

struct __vpiRealParam  : public __vpiRealConst {
      const char*basename;
      struct __vpiScope* scope;
      unsigned file_idx;
      unsigned lineno;
};

static int real_param_get(int code, vpiHandle ref)
{
      struct __vpiRealParam*rfp = (struct __vpiRealParam*)ref;

      assert(ref->vpi_type->type_code == vpiParameter);

      if (code == vpiLineNo) {
	    return rfp->lineno;
      }

      return real_get(code, ref);
}

static char* real_param_get_str(int code, vpiHandle obj)
{
      struct __vpiRealParam*rfp = (struct __vpiRealParam*)obj;

      assert(obj->vpi_type->type_code == vpiParameter);

      if (code == vpiFile) {
            return simple_set_rbuf_str(file_names[rfp->file_idx]);
      }

      return generic_get_str(code, &rfp->scope->base, rfp->basename, NULL);
}

static vpiHandle real_param_handle(int code, vpiHandle obj)
{
      struct __vpiRealParam*rfp = (struct __vpiRealParam*)obj;

      assert(obj->vpi_type->type_code == vpiParameter);

      switch (code) {
          case vpiScope:
            return &rfp->scope->base;

	  case vpiModule:
	    return vpip_module(rfp->scope);

          default:
            return 0;
      }
}

static const struct __vpirt vpip_real_param_rt = {
      vpiParameter,
      real_param_get,
      real_param_get_str,
      real_value,
      0,

      real_param_handle,
      0,
      0,

      0
};

vpiHandle vpip_make_real_param(char*name, double value,
                               long file_idx, long lineno)
{
      struct __vpiRealParam*obj;

      obj = (struct __vpiRealParam*)
            malloc(sizeof (struct __vpiRealParam));
      obj->base.vpi_type = &vpip_real_param_rt;
      obj->value = value;
      obj->basename = name;
      obj->scope = vpip_peek_current_scope();
      obj->file_idx = (unsigned) file_idx;
      obj->lineno = (unsigned) lineno;

      return &obj->base;
}

#ifdef CHECK_WITH_VALGRIND
void constant_delete(vpiHandle item)
{
      assert(item->vpi_type->type_code == vpiConstant);
      switch(vpi_get(vpiConstType, item)) {
	  case vpiStringConst: {
	    struct __vpiStringConst*rfp = (struct __vpiStringConst*)item;
	    delete [] rfp->value;
	    free(rfp);
	    break; }
	  case vpiDecConst: {
	    struct __vpiDecConst*rfp = (struct __vpiDecConst*)item;
	    free(rfp);
	    break; }
	  case vpiBinaryConst: {
	    struct __vpiBinaryConst*rfp = (struct __vpiBinaryConst*)item;
	    delete rfp;
	    break; }
	  case vpiRealConst: {
	    struct __vpiRealConst*rfp = (struct __vpiRealConst*)item;
	    free(rfp);
	    break; }
	  default:
	    assert(0);
      }
}

void parameter_delete(vpiHandle item)
{
      switch(vpi_get(vpiConstType, item)) {
	  case vpiStringConst: {
	    struct __vpiStringParam*rfp = (struct __vpiStringParam*)item;
	    delete [] rfp->basename;
	    delete [] rfp->value;
	    free(rfp);
	    break; }
	  case vpiBinaryConst: {
	    struct __vpiBinaryParam*rfp = (struct __vpiBinaryParam*)item;
	    delete [] rfp->basename;
	    delete rfp;
	    break; }
	  case vpiRealConst: {
	    struct __vpiRealParam*rfp = (struct __vpiRealParam*)item;
	    delete [] rfp->basename;
	    free(rfp);
	    break; }
	  default:
	    assert(0);
      }
}
#endif
