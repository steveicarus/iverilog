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
      assert(ref->vpi_type->type_code == vpiConstant);

      switch (code) {
	  case vpiConstType:
	    return vpiBinaryConst;

	  case vpiSigned:
	    return rfp->signed_flag? 1 : 0;

	  case vpiSize:
	    return rfp->nbits;

	  default:
	    fprintf(stderr, "vvp error: get %d not supported "
		    "by vpiBinaryConst\n", code);
	    assert(0);
	    return 0;
      }
}

static void binary_vpiStringVal(struct __vpiBinaryConst*rfp, p_vpi_value vp)
{
      unsigned nchar = rfp->nbits / 8;
      unsigned tail = rfp->nbits%8;

      char*rbuf = need_result_buf(nchar + 1, RBUF_VAL);
      char*cp = rbuf;

      if (tail > 0) {
	    char char_val = 0;
	    for (unsigned idx = rfp->nbits-tail; idx < rfp->nbits; idx += 1) {
		  unsigned nibble = idx/4;
		  unsigned shift  = 2 * (idx%4);
		  unsigned val = (rfp->bits[nibble] >> shift) & 3;
		  if (val & 1)
			char_val |= 1 << idx;
	    }

	    if (char_val != 0)
		  *cp++ = char_val;
      }

      for (unsigned idx = 0 ;  idx < nchar ;  idx += 1) {
	    unsigned bit = (nchar - idx - 1) * 8;
	    unsigned nibble = bit/4;
	    unsigned vall = rfp->bits[nibble+0];
	    unsigned valh = rfp->bits[nibble+1];

	    char char_val = 0;
	    if (vall&0x01) char_val |= 0x01;
	    if (vall&0x04) char_val |= 0x02;
	    if (vall&0x10) char_val |= 0x04;
	    if (vall&0x40) char_val |= 0x08;
	    if (valh&0x01) char_val |= 0x10;
	    if (valh&0x04) char_val |= 0x20;
	    if (valh&0x10) char_val |= 0x40;
	    if (valh&0x40) char_val |= 0x80;

	    if (char_val != 0)
		  *cp++ = char_val;
      }

      *cp = 0;
      vp->value.str = rbuf;
}

static int bits2int(struct __vpiBinaryConst*rfp)
{
      unsigned val = 0;
      unsigned bit_val = 0;
      unsigned bit_limit = rfp->nbits;
      if (bit_limit > 8*sizeof(val))
	bit_limit = 8*sizeof(val);

      for (unsigned idx = 0 ;  idx < bit_limit ;  idx += 1) {
	unsigned nibble = idx/4;
	unsigned shift  = 2 * (idx%4);
	bit_val = (rfp->bits[nibble] >> shift) & 3;
	if (bit_val > 1) {
	      return 0;
	} else {
	      val |= bit_val << idx;
	}
      }

      /* sign extend */
      if (rfp->signed_flag && bit_val) {
	  for (unsigned idx = rfp->nbits; idx <sizeof(val)*8; idx++)
	  {
	  val |= bit_val << idx;
	  }
      }

      return val;
}

static void binary_value(vpiHandle ref, p_vpi_value vp)
{
      assert(ref->vpi_type->type_code == vpiConstant);

      struct __vpiBinaryConst*rfp = (struct __vpiBinaryConst*)ref;
      char*rbuf = 0;


      switch (vp->format) {

	  case vpiObjTypeVal:
	  case vpiBinStrVal: {
	    rbuf = need_result_buf(rfp->nbits + 1, RBUF_VAL);
	    for (unsigned idx = 0 ;  idx < rfp->nbits ;  idx += 1) {
		  unsigned nibble = idx/4;
		  unsigned shift  = 2 * (idx%4);
		  unsigned val = (rfp->bits[nibble] >> shift) & 3;

		  rbuf[rfp->nbits-idx-1] = "01xz"[val];
	    }
	    rbuf[rfp->nbits] = 0;
	    vp->value.str = rbuf;
	    break;
	  }

	  case vpiDecStrVal: {
		unsigned wid = rfp->nbits;
		rbuf = need_result_buf(rfp->nbits + 1, RBUF_VAL);
		unsigned char*tmp = new unsigned char[wid];
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		      tmp[idx] = (rfp->bits[idx/4] >> 2*(idx%4)) & 3;

		vpip_bits_to_dec_str(tmp, wid, rbuf, wid + 1,
				     rfp->signed_flag);

		delete[]tmp;
		vp->value.str = rbuf;
		break;
	  }

	  case vpiHexStrVal: {
		unsigned nchar = (rfp->nbits+3)/4;
	        rbuf = need_result_buf(nchar + 1, RBUF_VAL);
		for (unsigned idx = 0 ;  idx < rfp->nbits ;  idx += 4) {
		      unsigned nibble = idx/4;
		      unsigned vals = rfp->bits[nibble];

		      if (vals == 0xff) {
			    rbuf[nchar-idx/4-1] = 'z';
		      } else if (vals == 0xaa) {
			    rbuf[nchar-idx/4-1] = 'x';
		      } else if (vals & 0xaa) {
			    rbuf[nchar-idx/4-1] = 'X';
		      } else {
			    unsigned val = vals&1;
			    if (vals&0x04) val |= 2;
			    if (vals&0x10) val |= 4;
			    if (vals&0x40) val |= 8;
			    rbuf[nchar-idx/4-1] = "0123456789abcdef"[val];
		      }
		}

		rbuf[nchar] = 0;
		vp->value.str = rbuf;
		break;
	  }

	  case vpiOctStrVal: {
		unsigned nchar = (rfp->nbits+2)/3;
	        rbuf = need_result_buf(nchar + 1, RBUF_VAL);
		vpip_bits_to_oct_str(rfp->bits, rfp->nbits,
				     rbuf, nchar+1, rfp->signed_flag);
		vp->value.str = rbuf;
		break;
	  }

	  case vpiIntVal: {
		vp->value.integer = bits2int(rfp);
		break;
	  }

	  case vpiVectorVal: {
	      unsigned int obit = 0;
	      unsigned hwid = (rfp->nbits - 1)/32 + 1;
	      rbuf = need_result_buf(hwid*sizeof(s_vpi_vecval), RBUF_VAL);

	      s_vpi_vecval *op = (p_vpi_vecval)rbuf;
	      vp->value.vector = op;

	      op->aval = op->bval = 0;
	      for (unsigned idx = 0 ;  idx < rfp->nbits ;  idx += 1) {
		unsigned nibble = idx/4;
		unsigned shift  = 2 * (idx%4);
		unsigned bit_val = (rfp->bits[nibble] >> shift) & 3;

		switch (bit_val) {
		case 0:
		  op->aval &= ~(1 << obit);
		  op->bval &= ~(1 << obit);
		  break;
		case 1:
		  op->aval |= (1 << obit);
		  op->bval &= ~(1 << obit);
		  break;
		case 2:
		  op->aval |= (1 << obit);
		  op->bval |= (1 << obit);
		  break;
		case 3:
		  op->aval &= ~(1 << obit);
		  op->bval |= (1 << obit);
		  break;
		}
		obit++;
		if (!(obit % 32)) {
		      op += 1;
		      if ((op - vp->value.vector) < (long)hwid)
			    op->aval = op->bval = 0;
		      obit = 0;
		}
	      }
	      break;
	  }

	  case vpiRealVal:
	      vp->value.real = (double)bits2int(rfp);
	    break;

	  case vpiStringVal:
	    binary_vpiStringVal(rfp, vp);
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

      obj = (struct __vpiBinaryConst*)
	    malloc(sizeof (struct __vpiBinaryConst));
      obj->base.vpi_type = &vpip_binary_rt;

      obj->signed_flag = 0;
      obj->nbits = wid;
      obj->bits = (unsigned char*)malloc((obj->nbits + 3) / 4);
      memset(obj->bits, 0, (obj->nbits + 3) / 4);

      const char*bp = bits;
      if (*bp == 's') {
	    bp += 1;
	    obj->signed_flag = 1;
      }

      for (unsigned idx = 0 ;  idx < obj->nbits ;  idx += 1) {
	    unsigned nibble = idx / 4;
	    unsigned val = 0;
	    switch (bp[wid-idx-1]) {
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
