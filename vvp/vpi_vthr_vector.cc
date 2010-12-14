/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
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

/*
 * vpiReg handles are handled here. These objects represent vectors of
 * .var objects that can be manipulated by the VPI module.
 */

# include  "vpi_priv.h"
# include  "vthread.h"
# include  <stdio.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <assert.h>

struct __vpiVThrVec {
      struct __vpiHandle base;
      unsigned bas;
      unsigned wid;
      unsigned signed_flag : 1;
      char *name;
};

inline static
unsigned get_bit(struct __vpiVThrVec *rfp, unsigned idx)
{
      return vthread_get_bit(vpip_current_vthread, rfp->bas+idx);
}

inline static
void set_bit(struct __vpiVThrVec *rfp, unsigned idx, unsigned bit)
{
      return vthread_put_bit(vpip_current_vthread, rfp->bas+idx, bit);
}


/*
 * Hex digits that represent 4-value bits of Verilog are not as
 * trivially obvious to display as if the bits were the usual 2-value
 * bits. So, although it is possible to write a function that
 * generates a correct character for 4*4-value bits, it is easier to
 * just perform the lookup in a table. This only takes 256 bytes,
 * which is not many executable instructions:-)
 *
 * The table is calculated as compile time, therefore, by the
 * draw_tt.c program.
 */

extern const char hex_digits[256];

extern const char oct_digits[256];

/*
 *  vpi_get
 */
static int vthr_vec_get(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg)
	     || (ref->vpi_type->type_code==vpiConstant));

      struct __vpiVThrVec*rfp = (struct __vpiVThrVec*)ref;

      switch (code) {

	  case vpiSigned:
	    return rfp->signed_flag;

	  case vpiSize:
	    return rfp->wid;

	  default:
	    return 0;
      }
}

static char* vthr_vec_get_str(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      struct __vpiVThrVec*rfp = (struct __vpiVThrVec*)ref;

      switch (code) {

	  case vpiFullName:
	    return (char*)rfp->name;
      }

      return 0;
}

static void vthr_vec_DecStrVal(struct __vpiVThrVec*rfp, s_vpi_value*vp)
{
      unsigned char*bits = new unsigned char[rfp->wid];
      char *rbuf = need_result_buf((rfp->wid+2)/3 + 1, RBUF_VAL);

      for (unsigned idx = 0 ;  idx < rfp->wid ;  idx += 1)
	    bits[idx] = get_bit(rfp, idx);

      vpip_bits_to_dec_str(bits, rfp->wid, rbuf, rfp->wid+1, rfp->signed_flag);
      vp->value.str = rbuf;

      return;
}

static void vthr_vec_StringVal(struct __vpiVThrVec*rfp, s_vpi_value*vp)
{
    char tmp = 0;
    char *rbuf = need_result_buf((rfp->wid / 8) + 1, RBUF_VAL);
    char *cp = rbuf;

    for(int bitnr=rfp->wid-1; bitnr>=0; bitnr--){
	tmp <<= 1;

	switch(get_bit(rfp, bitnr)){
	  case 0:
	      break;
	  case 1:
	      tmp |= 1;
	      break;
	  default:
	      break;
	}

	if ((bitnr&7)==0){
		// Don't including leading nulls
	      if (tmp == 0 && cp == rbuf)
		    continue;

		// Translated embedded nulls to space.
	      *cp++ = tmp? tmp : ' ';
	      tmp = 0;
	}
    }
    *cp++ = 0;
    vp->value.str = rbuf;
    return;
}

/*
 * The get_value method reads the values of the functors and returns
 * the vector to the caller. This causes no side-effect, and reads the
 * variables like a %load would.
 */
static void vthr_vec_get_value(vpiHandle ref, s_vpi_value*vp)
{
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg)
	     || (ref->vpi_type->type_code==vpiConstant));

      struct __vpiVThrVec*rfp = (struct __vpiVThrVec*)ref;
      char *rbuf;

      unsigned wid = rfp->wid;

      switch (vp->format) {

	  case vpiBinStrVal:
	    rbuf = need_result_buf(wid+1, RBUF_VAL);
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  rbuf[wid-idx-1] = "01xz"[get_bit(rfp, idx)];
	    }
	    rbuf[wid] = 0;
	    vp->value.str = rbuf;
	    break;

	  case vpiHexStrVal: {
		unsigned hval, hwid;
		hwid = (wid + 3) / 4;
		rbuf = need_result_buf(hwid+1, RBUF_VAL);
		rbuf[hwid] = 0;
		hval = 0;
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      hval = hval | (get_bit(rfp, idx) << 2*(idx % 4));

		      if (idx%4 == 3) {
			    hwid -= 1;
			    rbuf[hwid] = hex_digits[hval];
			    hval = 0;
		      }
		}

		if (hwid > 0) {
		      hwid -= 1;
		      rbuf[hwid] = hex_digits[hval];
		      hval = 0;
		}
		vp->value.str = rbuf;
		break;
	  }

	  case vpiOctStrVal: {
		unsigned hval, hwid;
		hwid = (wid + 2) / 3;
		rbuf = need_result_buf(hwid+1, RBUF_VAL);
		rbuf[hwid] = 0;
		hval = 0;
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      hval = hval | (get_bit(rfp,idx) << 2*(idx % 3));

		      if (idx%3 == 2) {
			    hwid -= 1;
			    rbuf[hwid] = oct_digits[hval];
			    hval = 0;
		      }
		}

		if (hwid > 0) {
		      hwid -= 1;
		      rbuf[hwid] = oct_digits[hval];
		      hval = 0;
		}
		vp->value.str = rbuf;
		break;
	  }

	  case vpiDecStrVal:
	    vthr_vec_DecStrVal(rfp, vp);
	    break;

	  case vpiStringVal:
	    vthr_vec_StringVal(rfp, vp);
	    break;

	  case vpiIntVal:
	    vp->value.integer = 0;
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  switch (get_bit(rfp, idx)) {
		      case 0:
			break;
		      case 1:
			vp->value.integer |= 1 << idx;
			break;
		      case 2:
		      case 3:
			break;
		  }
	    }
	    break;

	  case vpiRealVal:
	    vp->value.real = 0;
	    for (unsigned idx = wid ;  idx > 0 ;  idx -= 1) {
		  vp->value.real *= 2.0;
		  switch (get_bit(rfp, idx-1)) {
		      case 0:
			break;
		      case 1:
			vp->value.real += 1.0;
			break;
		      case 2:
		      case 3:
			break;
		  }
	    }
	    break;

	  case vpiVectorVal:
	    vp->value.vector = (s_vpi_vecval*)
		  need_result_buf((wid+31)/32*sizeof(s_vpi_vecval), RBUF_VAL);
	    assert(vp->value.vector);

	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  int word = idx/32;
		  PLI_INT32 mask = 1 << (idx%32);

		  switch (get_bit(rfp,idx)) {
		      case 0:
			vp->value.vector[word].aval &= ~mask;
			vp->value.vector[word].bval &= ~mask;
			break;
		      case 1:
			vp->value.vector[word].aval |=  mask;
			vp->value.vector[word].bval &= ~mask;
			break;
		      case 2:
			vp->value.vector[word].aval |=  mask;
			vp->value.vector[word].bval |=  mask;
			break;
		      case 3:
			vp->value.vector[word].aval &= ~mask;
			vp->value.vector[word].bval |=  mask;
			break;
		  }
	    }
	    break;

	  default:
	    fprintf(stderr, "internal error: vpi_get_value(<format=%d>)"
		    " not implemented for vthr_vectors.\n", vp->format);
	      /* XXXX Not implemented yet. */
	    assert(0);
      }
}

/*
 * The put_value method writes the value into the vector.
 */
static vpiHandle vthr_vec_put_value(vpiHandle ref, s_vpi_value*vp)
{
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      struct __vpiVThrVec*rfp = (struct __vpiVThrVec*)ref;

      unsigned wid = rfp->wid;

      switch (vp->format) {

	  case vpiIntVal: {
		assert(wid <= sizeof(long));

		long val = vp->value.integer;
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      set_bit(rfp, idx, val&1);
		      val >>= 1;
		}
		break;
	  }

	  case vpiScalarVal:
	    switch (vp->value.scalar) {
		case vpi0:
		  set_bit(rfp, 0, 0);
		  break;
		case vpi1:
		  set_bit(rfp, 0, 1);
		  break;
		case vpiX:
		  set_bit(rfp, 0, 2);
		  break;
		case vpiZ:
		  set_bit(rfp, 0, 3);
		  break;
		default:
		  assert(0);
	    }
	    break;

	  case vpiVectorVal: {
		assert(wid <= sizeof (unsigned long));

		unsigned long aval = vp->value.vector->aval;
		unsigned long bval = vp->value.vector->bval;
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      int bit = (aval&1) | (((bval^aval)<<1)&2);
		      set_bit(rfp, idx, bit);
		      aval >>= 1;
		      bval >>= 1;
		}
		break;
	  }

	  default:
	    assert(0);

      }

      return ref;
}

// The code fully supports vpiReg, vpi_Net, but we do not
// create such things, yet.  Lacking a name, for example.

static const struct __vpirt vpip_vthr_const_rt = {
      vpiConstant,
      vthr_vec_get,
      vthr_vec_get_str,
      vthr_vec_get_value,
      vthr_vec_put_value,
      0,
      0
};

/*
 * Construct a vpiReg object. Give the object specified dimensions,
 * and point to the specified functor for the lsb.
 */
vpiHandle vpip_make_vthr_vector(unsigned base, unsigned wid, bool signed_flag)
{
      struct __vpiVThrVec*obj = (struct __vpiVThrVec*)
	    malloc(sizeof(struct __vpiVThrVec));
      obj->base.vpi_type = &vpip_vthr_const_rt;
      assert(base < 65536);
      obj->bas = base;
      assert(wid < 65536);
      obj->wid = wid;
      obj->signed_flag = signed_flag? 1 : 0;
      obj->name = "T<>";

      return &obj->base;
}

struct __vpiVThrWord {
      struct __vpiHandle base;
      char* name;
      int subtype;
      unsigned index;
};

static int vthr_word_get(int code, vpiHandle ref)
{
      assert(ref->vpi_type->type_code==vpiConstant);

      struct __vpiVThrWord*rfp = (struct __vpiVThrWord*)ref;

      switch (code) {

	  case vpiConstType:
	    return rfp->subtype;

	  default:
	    return 0;
      }
}

static void vthr_real_get_value(vpiHandle ref, s_vpi_value*vp)
{
      assert(ref->vpi_type->type_code==vpiConstant);

      struct __vpiVThrWord*obj = (struct __vpiVThrWord*)ref;
      char *rbuf = need_result_buf(66, RBUF_VAL);

      double val = 0.0;

	/* Get the actual value from the index. It is possible, by the
	   way, that the vpi_get_value is called from compiletf. If
	   that's the case, there will be no current thread, and this
	   will not have access to the proper value. Punt and return a
	   0.0 value instead. */
      if (vpip_current_vthread)
	    val = vthread_get_real(vpip_current_vthread, obj->index);

      switch (vp->format) {

	  case vpiObjTypeVal:
	    vp->format = vpiRealVal;
	  case vpiRealVal:
	    vp->value.real = val;
	    break;

	  case vpiIntVal:
	    vp->value.integer = (int)(val + 0.5);
	    break;

	  case vpiDecStrVal:
	    sprintf(rbuf, "%0.0f", val);
	    vp->value.str = rbuf;
	    break;

	  case vpiHexStrVal:
	    sprintf(rbuf, "%lx", (long)val);
	    vp->value.str = rbuf;
	    break;

	  case vpiBinStrVal: {
		unsigned long vali = (unsigned long)val;
		unsigned len = 0;

		while (vali > 0) {
		      len += 1;
		      vali /= 2;
		}

		vali = (unsigned long)val;
		for (unsigned idx = 0 ;  idx < len ;  idx += 1) {
		      rbuf[len-idx-1] = (vali & 1)? '1' : '0';
		      vali /= 2;
		}

		rbuf[len] = 0;
		if (len == 0) {
		      rbuf[0] = '0';
		      rbuf[1] = 0;
		}
		vp->value.str = rbuf;
		break;
	  }

	  default:
	    fprintf(stderr, "vvp error: get %d not supported "
		      "by vpiConstant (Real)\n", vp->format);

	    vp->format = vpiSuppressVal;
	    break;
      }
}

static const struct __vpirt vpip_vthr_const_real_rt = {
      vpiConstant,
      vthr_word_get,
      0,
      vthr_real_get_value,
      0,
      0,
      0
};

vpiHandle vpip_make_vthr_word(unsigned base, const char*type)
{
      struct __vpiVThrWord*obj = (struct __vpiVThrWord*)
	    malloc(sizeof(struct __vpiVThrWord));

      assert(type[0] == 'r');

      obj->base.vpi_type = &vpip_vthr_const_real_rt;
      obj->name = "W<>";
      obj->subtype = vpiRealConst;
      assert(base < 65536);
      obj->index = base;

      return &obj->base;
}
