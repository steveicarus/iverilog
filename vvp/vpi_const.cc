/*
 * Copyright (c) 2001-2012 Stephen Williams (steve@icarus.com)
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
# include  "ivl_alloc.h"

static int string_get(int code, vpiHandle ref)
{
    struct __vpiStringConst*rfp;

      switch (code) {
          case vpiSize:
	    rfp = dynamic_cast<__vpiStringConst*>(ref);
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
      struct __vpiStringConst*rfp = dynamic_cast<__vpiStringConst*>(ref);
      int size = strlen(rfp->value);
      char*rbuf = 0;
      char*cp;

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

inline __vpiStringConst::__vpiStringConst()
{ }

int __vpiStringConst::get_type_code(void) const
{ return vpiConstant; }

int __vpiStringConst::vpi_get(int code)
{ return string_get(code, this); }

char*__vpiStringConst::vpi_get_str(int)
{ return 0; }

void __vpiStringConst::vpi_get_value(p_vpi_value val)
{ string_value(this, val); }

vpiHandle __vpiStringConst::vpi_put_value(p_vpi_value, int)
{ return 0; }

vpiHandle __vpiStringConst::vpi_handle(int)
{ return 0; }

vpiHandle __vpiStringConst::vpi_iterate(int)
{ return 0; }

vpiHandle __vpiStringConst::vpi_index(int)
{ return 0; }

void __vpiStringConst::vpi_get_delays(p_vpi_delay)
{ }

void __vpiStringConst::vpi_put_delays(p_vpi_delay)
{ }

struct __vpiStringConstTEMP : public __vpiStringConst {
      __vpiStringConstTEMP();
      free_object_fun_t free_object_fun(void);
};

static int free_temp_string(vpiHandle obj)
{
      struct __vpiStringConstTEMP*rfp = dynamic_cast<__vpiStringConstTEMP*>(obj);

      delete [] rfp->value;
      free(rfp);
      return 1;
}


inline __vpiStringConstTEMP::__vpiStringConstTEMP()
{ }
__vpiHandle::free_object_fun_t __vpiStringConstTEMP::free_object_fun(void)
{ return &free_temp_string; }

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

      obj = persistent_flag? new __vpiStringConst : new __vpiStringConstTEMP;
      obj->value = text;
      obj->value_len = 0;
      vpip_process_string(obj);

      return obj;
}


struct __vpiStringParam  : public __vpiStringConst {
      __vpiStringParam();
      int get_type_code(void) const;
      int vpi_get(int code);
      char*vpi_get_str(int code);
      vpiHandle vpi_handle(int code);

      const char*basename;
      struct __vpiScope* scope;
      unsigned file_idx;
      unsigned lineno;
};

static int string_param_get(int code, vpiHandle ref)
{
      struct __vpiStringParam*rfp = dynamic_cast<__vpiStringParam*>(ref);
      assert(ref);

      if (code == vpiLineNo) {
	    return rfp->lineno;
      }

      return string_get(code, ref);
}

static char* string_param_get_str(int code, vpiHandle obj)
{
      struct __vpiStringParam*rfp = dynamic_cast<__vpiStringParam*>(obj);
      assert(rfp);

      if (code == vpiFile) {
	    return simple_set_rbuf_str(file_names[rfp->file_idx]);
      }

      return generic_get_str(code, rfp->scope, rfp->basename, NULL);
}

static vpiHandle string_param_handle(int code, vpiHandle obj)
{
      struct __vpiStringParam*rfp = dynamic_cast<__vpiStringParam*>(obj);
      assert(rfp);

      switch (code) {
	  case vpiScope:
	    return rfp->scope;

	  case vpiModule:
	    return vpip_module(rfp->scope);

	  default:
	    return 0;
      }
}


inline __vpiStringParam::__vpiStringParam()
{ }

int __vpiStringParam::get_type_code(void) const
{ return vpiParameter; }

int __vpiStringParam::vpi_get(int code)
{ return string_param_get(code, this); }

char*__vpiStringParam::vpi_get_str(int code)
{ return string_param_get_str(code, this); }

vpiHandle __vpiStringParam::vpi_handle(int code)
{ return string_param_handle(code, this); }

vpiHandle vpip_make_string_param(char*name, char*text,
                                 long file_idx, long lineno)
{
      struct __vpiStringParam*obj = new __vpiStringParam;
      obj->value = text;
      obj->value_len = 0;
      obj->basename = name;
      obj->scope = vpip_peek_current_scope();
      obj->file_idx = (unsigned) file_idx;
      obj->lineno = (unsigned) lineno;

      vpip_process_string(obj);

      return obj;
}

static int binary_get(int code, vpiHandle ref)
{
      struct __vpiBinaryConst*rfp = dynamic_cast<__vpiBinaryConst*>(ref);

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
      struct __vpiBinaryConst*rfp = dynamic_cast<__vpiBinaryConst*>(ref);

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


inline __vpiBinaryConst::__vpiBinaryConst()
{ }

int __vpiBinaryConst::get_type_code(void) const
{ return vpiConstant; }

int __vpiBinaryConst::vpi_get(int code)
{ return binary_get(code, this); }

char* __vpiBinaryConst::vpi_get_str(int)
{ return 0; }

void __vpiBinaryConst::vpi_get_value(p_vpi_value val)
{ binary_value(this, val); }

vpiHandle __vpiBinaryConst::vpi_put_value(p_vpi_value, int)
{ return 0; }

vpiHandle __vpiBinaryConst::vpi_handle(int)
{ return 0; }

vpiHandle __vpiBinaryConst::vpi_iterate(int)
{ return 0; }

vpiHandle __vpiBinaryConst::vpi_index(int)
{ return 0; }

void __vpiBinaryConst::vpi_get_delays(p_vpi_delay)
{ }

void __vpiBinaryConst::vpi_put_delays(p_vpi_delay)
{ }

/*
 * Make a VPI constant from a vector string. The string is normally a
 * ASCII string, with each letter a 4-value bit. The first character
 * may be an 's' if the vector is signed.
 */
vpiHandle vpip_make_binary_const(unsigned wid, const char*bits)
{
      struct __vpiBinaryConst*obj = new __vpiBinaryConst;

      obj->signed_flag = 0;
      obj->sized_flag = 0;

      const char*bp = bits;
      if (*bp == 's') {
	    bp += 1;
	    obj->signed_flag = 1;
      }

      obj->bits = vector4_from_text(bp, wid);

      return obj;
}

vvp_vector4_t vector4_from_text(const char*bits, unsigned wid)
{
      vvp_vector4_t res (wid);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_bit4_t val = BIT4_0;
	    switch (bits[wid-idx-1]) {
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

	    res.set_bit(idx, val);
      }

      return res;
}

struct __vpiBinaryParam  : public __vpiBinaryConst {
      __vpiBinaryParam();
      int get_type_code(void) const;
      int vpi_get(int code);
      char*vpi_get_str(int code);
      vpiHandle vpi_handle(int code);

      const char*basename;
      struct __vpiScope*scope;
      unsigned file_idx;
      unsigned lineno;
};

static int binary_param_get(int code, vpiHandle ref)
{
      struct __vpiBinaryParam*rfp = dynamic_cast<__vpiBinaryParam*>(ref);

      if (code == vpiLineNo) {
	    return rfp->lineno;
      }

      return binary_get(code, ref);
}

static char* binary_param_get_str(int code, vpiHandle obj)
{
      struct __vpiBinaryParam*rfp = dynamic_cast<__vpiBinaryParam*>(obj);

      if (code == vpiFile) {
	    return simple_set_rbuf_str(file_names[rfp->file_idx]);
      }

      return generic_get_str(code, rfp->scope, rfp->basename, NULL);
}

static vpiHandle binary_param_handle(int code, vpiHandle obj)
{
      struct __vpiBinaryParam*rfp = dynamic_cast<__vpiBinaryParam*>(obj);

      switch (code) {
	  case vpiScope:
	    return rfp->scope;

	  case vpiModule:
	    return vpip_module(rfp->scope);

	  default:
	    return 0;
      }
}

inline __vpiBinaryParam::__vpiBinaryParam()
{ }

int __vpiBinaryParam::get_type_code(void) const
{ return vpiParameter; }

int __vpiBinaryParam::vpi_get(int code)
{ return binary_param_get(code, this); }

char*__vpiBinaryParam::vpi_get_str(int code)
{ return binary_param_get_str(code, this); }

vpiHandle __vpiBinaryParam::vpi_handle(int code)
{ return binary_param_handle(code, this); }

vpiHandle vpip_make_binary_param(char*name, const vvp_vector4_t&bits,
				 bool signed_flag,
				 long file_idx, long lineno)
{
      struct __vpiBinaryParam*obj = new __vpiBinaryParam;

      obj->bits = bits;
      obj->signed_flag = signed_flag? 1 : 0;
      obj->sized_flag = 0;
      obj->basename = name;
      obj->scope = vpip_peek_current_scope();
      obj->file_idx = (unsigned) file_idx;
      obj->lineno = (unsigned) lineno;

      return obj;
}


static int dec_get(int code, vpiHandle)
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
      struct __vpiDecConst*rfp = dynamic_cast<__vpiDecConst*>(ref);
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

__vpiDecConst::__vpiDecConst(int val)
{
      value = val;
}

int __vpiDecConst::get_type_code(void) const
{ return vpiConstant; }

int __vpiDecConst::vpi_get(int code)
{ return dec_get(code, this); }

void __vpiDecConst::vpi_get_value(p_vpi_value val)
{ dec_value(this, val); }

static int real_get(int code, vpiHandle)
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
      struct __vpiRealConst*rfp = dynamic_cast<__vpiRealConst*>(ref);
       vpip_real_get_value(rfp->value, vp);
}

inline __vpiRealConst::__vpiRealConst()
{ }

int __vpiRealConst::get_type_code(void) const
{ return vpiConstant; }

int __vpiRealConst::vpi_get(int code)
{ return real_get(code, this); }

void __vpiRealConst::vpi_get_value(p_vpi_value val)
{ real_value(this, val); }

vpiHandle vpip_make_real_const(double value)
{
      struct __vpiRealConst*obj = new __vpiRealConst;
      obj->value = value;
      return obj;
}

struct __vpiRealParam  : public __vpiRealConst {
      __vpiRealParam();
      int get_type_code(void) const;
      int vpi_get(int code);
      char*vpi_get_str(int code);
      vpiHandle vpi_handle(int code);
 
      const char*basename;
      struct __vpiScope* scope;
      unsigned file_idx;
      unsigned lineno;
};

static int real_param_get(int code, vpiHandle ref)
{
      struct __vpiRealParam*rfp = dynamic_cast<__vpiRealParam*>(ref);

      if (code == vpiLineNo) {
	    return rfp->lineno;
      }

      return real_get(code, ref);
}

static char* real_param_get_str(int code, vpiHandle obj)
{
      struct __vpiRealParam*rfp = dynamic_cast<__vpiRealParam*>(obj);

      if (code == vpiFile) {
            return simple_set_rbuf_str(file_names[rfp->file_idx]);
      }

      return generic_get_str(code, rfp->scope, rfp->basename, NULL);
}

static vpiHandle real_param_handle(int code, vpiHandle obj)
{
      struct __vpiRealParam*rfp = dynamic_cast<__vpiRealParam*>(obj);

      switch (code) {
          case vpiScope:
            return rfp->scope;

	  case vpiModule:
	    return vpip_module(rfp->scope);

          default:
            return 0;
      }
}


inline __vpiRealParam::__vpiRealParam()
{ }

int __vpiRealParam::get_type_code(void) const
{ return vpiParameter; }

int __vpiRealParam::vpi_get(int code)
{ return real_param_get(code, this); }

char* __vpiRealParam::vpi_get_str(int code)
{ return real_param_get_str(code, this); }

vpiHandle __vpiRealParam::vpi_handle(int code)
{ return real_param_handle(code, this); }
 

vpiHandle vpip_make_real_param(char*name, double value,
                               long file_idx, long lineno)
{
      struct __vpiRealParam*obj = new __vpiRealParam;

      obj->value = value;
      obj->basename = name;
      obj->scope = vpip_peek_current_scope();
      obj->file_idx = (unsigned) file_idx;
      obj->lineno = (unsigned) lineno;

      return obj;
}

#ifdef CHECK_WITH_VALGRIND
void constant_delete(vpiHandle item)
{
      assert(item->vpi_type->type_code == vpiConstant);
      switch(vpi_get(vpiConstType, item)) {
	  case vpiStringConst: {
	    struct __vpiStringConst*rfp = dynamic_cast<__vpiStringConst*>(item);
	    delete [] rfp->value;
	    free(rfp);
	    break; }
	  case vpiDecConst: {
	    struct __vpiDecConst*rfp = dynamic_cast<__vpiDecConst*>(item);
	    free(rfp);
	    break; }
	  case vpiBinaryConst: {
	    struct __vpiBinaryConst*rfp = dynamic_cast<__vpiBinaryConst*>(item);
	    delete rfp;
	    break; }
	  case vpiRealConst: {
	    struct __vpiRealConst*rfp = dynamic_cast<__vpiRealConst*>(item);
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
	    struct __vpiStringParam*rfp = dynamic_cast<__vpiStringParam*>(item);
	    delete [] rfp->basename;
	    delete [] rfp->value;
	    free(rfp);
	    break; }
	  case vpiBinaryConst: {
	    struct __vpiBinaryParam*rfp = dynamic_cast<__vpiBinaryParam*>(item);
	    delete [] rfp->basename;
	    delete rfp;
	    break; }
	  case vpiRealConst: {
	    struct __vpiRealParam*rfp = dynamic_cast<__vpiRealParam*>(item);
	    delete [] rfp->basename;
	    free(rfp);
	    break; }
	  default:
	    assert(0);
      }
}
#endif
