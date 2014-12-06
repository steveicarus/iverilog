/*
 * Copyright (c) 2001-2014 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

class __vpiStringConst : public __vpiHandle {
    public:
      __vpiStringConst(char*val);
      ~__vpiStringConst();
      int get_type_code(void) const;
      int vpi_get(int code);
      void vpi_get_value(p_vpi_value val);

    private:
      void process_string_();
    private:
      char*value_;
      size_t value_len_;
};

inline __vpiStringConst::__vpiStringConst(char*v)
: value_(v)
{
      process_string_();
}

/*
 * Strings are described at the level of the vvp source as a string
 * with literal characters or octal escapes. No other escapes are
 * included, they are processed already by the compiler that generated
 * the vvp source.
 */
void __vpiStringConst::process_string_(void)
{
      char*chr = value_;
      char*dp = value_;

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
      value_len_ = dp - value_;
}

__vpiStringConst::~__vpiStringConst()
{
      delete[] value_;
}

int __vpiStringConst::get_type_code(void) const
{ return vpiConstant; }

int __vpiStringConst::vpi_get(int code)
{
      switch (code) {
          case vpiSize:
	    return strlen(value_)*8;

          case vpiSigned:
	      return 0;

	  case vpiConstType:
	      return vpiStringConst;

          case vpiAutomatic:
	      return 0;

#if defined(CHECK_WITH_VALGRIND) || defined(BR916_STOPGAP_FIX)
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

void __vpiStringConst::vpi_get_value(p_vpi_value vp)
{
      unsigned uint_value;
      p_vpi_vecval vecp;
      unsigned size = strlen(value_);
      char*rbuf = 0;
      char*cp;

      switch (vp->format) {
	  case vpiObjTypeVal:
	      /* String parameters by default have vpiStringVal values. */
	    vp->format = vpiStringVal;

	  case vpiStringVal:
	    rbuf = (char *) need_result_buf(size + 1, RBUF_VAL);
	    strcpy(rbuf, value_);
	    vp->value.str = rbuf;
	    break;

          case vpiDecStrVal:
	      if (size > 4){
		  // We only support standard integers. Ignore other bytes...
		  size = 4;
		  fprintf(stderr, "Warning (vpi_const.cc): %%d on constant strings only looks "
			  "at first 4 bytes!\n");
	      }
	      rbuf = (char *) need_result_buf(size + 1, RBUF_VAL);
	      uint_value = 0;
	      for(unsigned i=0; i<size; i += 1){
		  uint_value <<=8;
		  uint_value += (unsigned char)(value_[i]);
	      }
	      sprintf(rbuf, "%u", uint_value);
	      vp->value.str = rbuf;
	      break;

          case vpiBinStrVal:
	      rbuf = (char *) need_result_buf(8 * size + 1, RBUF_VAL);
	      cp = rbuf;
	      for(unsigned i=0; i<size; i += 1){
		  for(int bit=7; bit>=0; bit -= 1){
		      *cp++ = "01"[ (value_[i]>>bit)&1 ];
		  }
	      }
	      *cp = 0;
	      vp->value.str = rbuf;
	      break;

          case vpiHexStrVal:
	      rbuf = (char *) need_result_buf(2 * size + 1, RBUF_VAL);
	      cp = rbuf;
	      for(unsigned i=0; i<size; i += 1){
		  for(int nibble=1; nibble>=0; nibble -= 1){
		      *cp++ = "0123456789abcdef"[ (value_[i]>>(nibble*4))&15 ];
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
	      for(unsigned i=0; i<size; i += 1){
		  for(int bit=7; bit>=0; bit -= 1){
		      vp->value.integer <<= 1;
		      vp->value.integer += (value_[i]>>bit)&1;
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
	      for(unsigned i=0; i<size; i += 1){
		  vecp->aval |= value_[i] << uint_value*8;
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


struct __vpiStringConstTEMP : public __vpiStringConst {
      inline __vpiStringConstTEMP(char*v) : __vpiStringConst(v) { }
      free_object_fun_t free_object_fun(void);
};

static int free_temp_string(vpiHandle obj)
{
      struct __vpiStringConstTEMP*rfp = dynamic_cast<__vpiStringConstTEMP*>(obj);
      delete rfp;
      return 1;
}

__vpiHandle::free_object_fun_t __vpiStringConstTEMP::free_object_fun(void)
{ return &free_temp_string; }

vpiHandle vpip_make_string_const(char*text, bool persistent_flag)
{
      __vpiStringConst*obj;

      obj = persistent_flag? new __vpiStringConst(text) : new __vpiStringConstTEMP(text);

      return obj;
}


class __vpiStringParam  : public __vpiStringConst {
    public:
      __vpiStringParam(char*txt, char*name);
      ~__vpiStringParam();
      int get_type_code(void) const;
      int vpi_get(int code);
      char*vpi_get_str(int code);
      vpiHandle vpi_handle(int code);

      struct __vpiScope* scope;
      bool     local_flag;
      unsigned file_idx;
      unsigned lineno;
    private:
      const char*basename_;
};

inline __vpiStringParam::__vpiStringParam(char*txt, char*nam)
: __vpiStringConst(txt)
{
      basename_ = nam;
}

__vpiStringParam::~__vpiStringParam()
{
      delete[]basename_;
}

int __vpiStringParam::get_type_code(void) const
{ return vpiParameter; }

int __vpiStringParam::vpi_get(int code)
{
    switch (code) {
       case vpiLineNo :
         return lineno;

       case vpiLocalParam :
         return local_flag;

       default :
         return __vpiStringConst::vpi_get(code);
    }
}


char*__vpiStringParam::vpi_get_str(int code)
{
      if (code == vpiFile) {
	    return simple_set_rbuf_str(file_names[file_idx]);
      }

      return generic_get_str(code, scope, basename_, NULL);
}


vpiHandle __vpiStringParam::vpi_handle(int code)
{
      switch (code) {
	  case vpiScope:
	    return scope;

	  case vpiModule:
	    return vpip_module(scope);

	  default:
	    return 0;
      }
}

vpiHandle vpip_make_string_param(char*name, char*text,
                                  bool local_flag, long file_idx, long lineno)
{
      __vpiStringParam*obj = new __vpiStringParam(text, name);
      obj->scope = vpip_peek_current_scope();
      obj->local_flag = local_flag;
      obj->file_idx = (unsigned) file_idx;
      obj->lineno = (unsigned) lineno;

      return obj;
}



inline __vpiBinaryConst::__vpiBinaryConst()
{ }

int __vpiBinaryConst::get_type_code(void) const
{ return vpiConstant; }

int __vpiBinaryConst::vpi_get(int code)
{
      switch (code) {
	  case vpiConstType:
	    return vpiBinaryConst;

	  case vpiLineNo:
	    return 0;  // Not implemented for now!

	  case vpiSigned:
	    return signed_flag? 1 : 0;

	  case vpiSize:
	    return bits.size();

          case vpiAutomatic:
	    return 0;

#if defined(CHECK_WITH_VALGRIND) || defined(BR916_STOPGAP_FIX)
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


void __vpiBinaryConst::vpi_get_value(p_vpi_value val)
{
      switch (val->format) {

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
	    vpip_vec4_get_value(bits, bits.size(), signed_flag, val);
	    break;

	  default:
	    fprintf(stderr, "vvp error: format %d not supported "
		    "by vpiBinaryConst\n", (int)val->format);
	    val->format = vpiSuppressVal;
	    break;
      }
}


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
      __vpiBinaryParam(const vvp_vector4_t&b, char*name);
      ~__vpiBinaryParam();
      int get_type_code(void) const;
      int vpi_get(int code);
      char*vpi_get_str(int code);
      vpiHandle vpi_handle(int code);

      struct __vpiScope*scope;
      unsigned file_idx;
      unsigned lineno;
      bool     local_flag;
    private:
      char*basename_;
};

inline __vpiBinaryParam::__vpiBinaryParam(const vvp_vector4_t&b, char*nam)
{
      bits = b;
      basename_ = nam;
}

__vpiBinaryParam::~__vpiBinaryParam()
{
      delete[]basename_;
}

int __vpiBinaryParam::get_type_code(void) const
{ return vpiParameter; }

int __vpiBinaryParam::vpi_get(int code)
{
    switch (code) {
      case vpiLineNo :
        return lineno;

      case vpiLocalParam :
        return local_flag;

      default :
        return __vpiBinaryConst::vpi_get(code);
    }
}

char*__vpiBinaryParam::vpi_get_str(int code)
{
      if (code == vpiFile)
	    return simple_set_rbuf_str(file_names[file_idx]);

      return generic_get_str(code, scope, basename_, NULL);
}


vpiHandle __vpiBinaryParam::vpi_handle(int code)
{
      switch (code) {
	  case vpiScope:
	    return scope;

	  case vpiModule:
	    return vpip_module(scope);

	  default:
	    return 0;
      }
}


vpiHandle vpip_make_binary_param(char*name, const vvp_vector4_t&bits,
				 bool signed_flag, bool local_flag,
				 long file_idx, long lineno)
{
      struct __vpiBinaryParam*obj = new __vpiBinaryParam(bits, name);

      obj->signed_flag = signed_flag? 1 : 0;
      obj->sized_flag = 0;
      obj->local_flag = local_flag;
      obj->scope = vpip_peek_current_scope();
      obj->file_idx = (unsigned) file_idx;
      obj->lineno = (unsigned) lineno;

      return obj;
}



__vpiDecConst::__vpiDecConst(int val)
{
      value = val;
}

__vpiDecConst::__vpiDecConst(const __vpiDecConst&that)
: __vpiHandle(), value(that.value)
{
}


int __vpiDecConst::get_type_code(void) const
{ return vpiConstant; }

int __vpiDecConst::vpi_get(int code)
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

#if defined(CHECK_WITH_VALGRIND) || defined(BR916_STOPGAP_FIX)
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


void __vpiDecConst::vpi_get_value(p_vpi_value vp)
{
      char*rbuf = (char *) need_result_buf(64 + 1, RBUF_VAL);
      char*cp = rbuf;

      switch (vp->format) {

	  case vpiObjTypeVal:
	  case vpiIntVal: {
		vp->value.integer = value;
		break;
	  }

          case vpiDecStrVal:
	      sprintf(rbuf, "%d", value);

	      vp->value.str = rbuf;
	      break;

          case vpiBinStrVal:
	      for(int bit=31; bit<=0;bit--){
		  *cp++ = "01"[ (value>>bit)&1 ];
	      }
	      *cp = 0;

	      vp->value.str = rbuf;
	      break;

          case vpiHexStrVal:
	      sprintf(rbuf, "%08x", value);

	      vp->value.str = rbuf;
	      break;

          case vpiOctStrVal:
	      sprintf(rbuf, "%011x", value);

	      vp->value.str = rbuf;
	      break;

	  default:
	    fprintf(stderr, "vvp error (vpi_const.cc): format %d not supported "
		    "by vpiDecConst\n", (int)vp->format);
	    vp->format = vpiSuppressVal;
	    break;
      }
}


inline __vpiRealConst::__vpiRealConst(double val)
: value(val)
{ }

int __vpiRealConst::get_type_code(void) const
{ return vpiConstant; }

int __vpiRealConst::vpi_get(int code)
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

#if defined(CHECK_WITH_VALGRIND) || defined(BR916_STOPGAP_FIX)
          case _vpiFromThr:
	      return _vpiNoThr;
#endif

	  default:
	    fprintf(stderr, "vvp error: get %d not supported "
		    "by vpiRealConst\n", code);
	    assert(0);
	    return 0;
      }
}


void __vpiRealConst::vpi_get_value(p_vpi_value val)
{
      vpip_real_get_value(value, val);
}


vpiHandle vpip_make_real_const(double value)
{
      __vpiRealConst*obj = new __vpiRealConst(value);
      return obj;
}

struct __vpiRealParam  : public __vpiRealConst {
      __vpiRealParam(double val, char*name);
      ~__vpiRealParam();
      int get_type_code(void) const;
      int vpi_get(int code);
      char*vpi_get_str(int code);
      vpiHandle vpi_handle(int code);

      struct __vpiScope* scope;
      bool local_flag;
      unsigned file_idx;
      unsigned lineno;
    private:
      const char*basename_;
};


inline __vpiRealParam::__vpiRealParam(double val, char*name)
: __vpiRealConst(val)
{
      basename_ = name;
}

__vpiRealParam::~__vpiRealParam()
{
      delete[]basename_;
}


int __vpiRealParam::get_type_code(void) const
{ return vpiParameter; }

int __vpiRealParam::vpi_get(int code)
{
    switch (code) {
      case vpiLineNo :
        return lineno;

      case vpiLocalParam :
        return local_flag;

      default :
           return __vpiRealConst::vpi_get(code);
    }
}

char* __vpiRealParam::vpi_get_str(int code)
{
      if (code == vpiFile)
            return simple_set_rbuf_str(file_names[file_idx]);

      return generic_get_str(code, scope, basename_, NULL);
}

vpiHandle __vpiRealParam::vpi_handle(int code)
{
      switch (code) {
          case vpiScope:
            return scope;

	  case vpiModule:
	    return vpip_module(scope);

          default:
            return 0;
      }
}


vpiHandle vpip_make_real_param(char*name, double value,
                                bool local_flag, long file_idx, long lineno)
{
      struct __vpiRealParam*obj = new __vpiRealParam(value, name);

      obj->scope = vpip_peek_current_scope();
      obj->local_flag = local_flag;
      obj->file_idx = (unsigned) file_idx;
      obj->lineno = (unsigned) lineno;

      return obj;
}

#ifdef CHECK_WITH_VALGRIND
void constant_delete(vpiHandle item)
{
      assert(item->get_type_code() == vpiConstant);
      switch(vpi_get(vpiConstType, item)) {
	  case vpiStringConst:
	    delete dynamic_cast<__vpiStringConst*>(item);
	    break;
	  case vpiDecConst:
	    delete dynamic_cast<__vpiDecConst*>(item);
	    break;
	  case vpiBinaryConst:
	    delete dynamic_cast<__vpiBinaryConst*>(item);
	    break;
	  case vpiRealConst:
	    delete dynamic_cast<__vpiRealConst*>(item);
	    break;
	  default:
	    assert(0);
      }
}

void parameter_delete(vpiHandle item)
{
      switch(vpi_get(vpiConstType, item)) {
	  case vpiStringConst: {
	    struct __vpiStringParam*rfp = dynamic_cast<__vpiStringParam*>(item);
	    delete rfp;
	    break; }
	  case vpiBinaryConst: {
	    struct __vpiBinaryParam*rfp = dynamic_cast<__vpiBinaryParam*>(item);
	    delete rfp;
	    break; }
	  case vpiRealConst: {
	    struct __vpiRealParam*rfp = dynamic_cast<__vpiRealParam*>(item);
	    delete rfp;
	    break; }
	  default:
	    assert(0);
      }
}
#endif
