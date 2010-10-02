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
# include  "config.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
# include <map>
#endif
# include  <cstdio>
# include  <cstdlib>
# include  <cassert>

struct __vpiVThrVec {
      struct __vpiHandle base;
      unsigned bas;
      unsigned wid;
      unsigned signed_flag : 1;
      const char *name;
};

inline static
vvp_bit4_t get_bit(struct __vpiVThrVec *rfp, unsigned idx)
{
      return vthread_get_bit(vpip_current_vthread, rfp->bas+idx);
}

inline static
void set_bit(struct __vpiVThrVec *rfp, unsigned idx, vvp_bit4_t bit)
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

extern const char oct_digits[64];

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

	  case vpiConstType:
	    return vpiBinaryConst; // If this is a constant it is Binary.

	  case vpiSize:
	    return rfp->wid;

#ifdef CHECK_WITH_VALGRIND
	  case _vpiFromThr:
	    return _vpiVThr;
#endif

	  default:
	    return 0;
      }
}

static char* vthr_vec_get_str(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg)
	     || (ref->vpi_type->type_code==vpiConstant));

      struct __vpiVThrVec*rfp = (struct __vpiVThrVec*)ref;

      switch (code) {

	  case vpiFullName:   /* should this be vpiName? */
	    return simple_set_rbuf_str(rfp->name);
      }

      return 0;
}

static void vthr_vec_DecStrVal(struct __vpiVThrVec*rfp, s_vpi_value*vp)
{
      int nbuf = (rfp->wid+2)/3 + 1;
      char *rbuf = need_result_buf(nbuf, RBUF_VAL);

      vvp_vector4_t tmp (rfp->wid);
      for (unsigned idx = 0 ;  idx < rfp->wid ;  idx += 1)
	    tmp.set_bit(idx, get_bit(rfp, idx));

      vpip_vec4_to_dec_str(tmp, rbuf, nbuf, rfp->signed_flag);
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
	  case BIT4_0:
	      break;
	  case BIT4_1:
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
		  rbuf[wid-idx-1] = vvp_bit4_to_ascii(get_bit(rfp, idx));
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
		      unsigned tmp = 0;
		      switch (get_bit(rfp, idx)) {
			  case BIT4_0:
			    tmp = 0;
			    break;
			  case BIT4_1:
			    tmp = 1;
			    break;
			  case BIT4_X:
			    tmp = 2;
			    break;
			  case BIT4_Z:
			    tmp = 3;
			    break;
		      }
		      hval = hval | (tmp << 2*(idx % 4));

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
		      unsigned tmp = 0;
		      switch (get_bit(rfp, idx)) {
			  case BIT4_0:
			    tmp = 0;
			    break;
			  case BIT4_1:
			    tmp = 1;
			    break;
			  case BIT4_X:
			    tmp = 2;
			    break;
			  case BIT4_Z:
			    tmp = 3;
			    break;
		      }
		      hval = hval | (tmp << 2*(idx % 3));

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

	  case vpiIntVal: {
	    long ival = 0;
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  switch (get_bit(rfp, idx)) {
		      case BIT4_0:
			break;
		      case BIT4_1:
			ival |= 1 << idx;
			break;
		      default:
			break;
		  }
	    }
	    vp->value.integer = ival;
	    }
	    break;

	  case vpiRealVal:
	    vp->value.real = 0;
	    for (unsigned idx = wid ;  idx > 0 ;  idx -= 1) {
		  vp->value.real *= 2.0;
		  switch (get_bit(rfp, idx-1)) {
		      case BIT4_0:
			break;
		      case BIT4_1:
			vp->value.real += 1.0;
			break;
		      default:
			break;
		  }
	    }
	    break;

	  case vpiObjTypeVal:
	    vp->format = vpiVectorVal;
	  case vpiVectorVal:
	    vp->value.vector = (s_vpi_vecval*)
		  need_result_buf((wid+31)/32*sizeof(s_vpi_vecval), RBUF_VAL);
	    assert(vp->value.vector);

	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  int word = idx/32;
		  PLI_INT32 mask = 1 << (idx%32);

		  switch (get_bit(rfp,idx)) {
		      case BIT4_0:
			vp->value.vector[word].aval &= ~mask;
			vp->value.vector[word].bval &= ~mask;
			break;
		      case BIT4_1:
			vp->value.vector[word].aval |=  mask;
			vp->value.vector[word].bval &= ~mask;
			break;
		      case BIT4_X:
			vp->value.vector[word].aval |=  mask;
			vp->value.vector[word].bval |=  mask;
			break;
		      case BIT4_Z:
			vp->value.vector[word].aval &= ~mask;
			vp->value.vector[word].bval |=  mask;
			break;
		  }
	    }
	    break;

	  default:
	    fprintf(stderr, "internal error: vpi_get_value(<format=%d>)"
		    " not implemented for vthr_vectors.\n", (int)vp->format);
	      /* XXXX Not implemented yet. */
	    assert(0);
      }
}

/*
 * The put_value method writes the value into the vector.
 */
static vpiHandle vthr_vec_put_value(vpiHandle ref, s_vpi_value*vp, int)
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
		      set_bit(rfp, idx, (val&1)? BIT4_1 : BIT4_0);
		      val >>= 1;
		}
		break;
	  }

	  case vpiScalarVal:
	    switch (vp->value.scalar) {
		case vpi0:
		  set_bit(rfp, 0, BIT4_0);
		  break;
		case vpi1:
		  set_bit(rfp, 0, BIT4_1);
		  break;
		case vpiX:
		  set_bit(rfp, 0, BIT4_X);
		  break;
		case vpiZ:
		  set_bit(rfp, 0, BIT4_Z);
		  break;
		default:
		  fprintf(stderr, "Unsupported scalar value %d.\n",
		          (int)vp->value.scalar);
		  assert(0);
	    }
	    break;

	  case vpiVectorVal: {
		assert(wid <= sizeof (unsigned long));

		unsigned long aval = vp->value.vector->aval;
		unsigned long bval = vp->value.vector->bval;
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      int bit = (aval&1) | (((bval^aval)<<1)&2);
		      switch (bit) {
			  case 0:
			    set_bit(rfp, idx, BIT4_0);
			    break;
			  case 1:
			    set_bit(rfp, idx, BIT4_1);
			    break;
			  case 2:
			    set_bit(rfp, idx, BIT4_X);
			    break;
			  case 3:
			    set_bit(rfp, idx, BIT4_Z);
			    break;
		      }
		      aval >>= 1;
		      bval >>= 1;
		}
		break;
	  }

	  default:
	    fprintf(stderr, "Unsupported format %d.\n", (int)vp->format);
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
      obj->name = vpip_name_string("T<>");

      return &obj->base;
}

#ifdef CHECK_WITH_VALGRIND
static map<vpiHandle, bool> handle_map;

void thread_vthr_delete(vpiHandle item)
{
      handle_map[item] = true;
}

static void thread_vthr_delete_real(vpiHandle item)
{
      struct __vpiVThrVec*obj = (struct __vpiVThrVec*)item;
      free (obj);
}
#endif

struct __vpiVThrWord {
      struct __vpiHandle base;
      const char* name;
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

#ifdef CHECK_WITH_VALGRIND
	  case _vpiFromThr:
	    return _vpiWord;
#endif

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
		      "by vpiConstant (Real)\n", (int)vp->format);

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
      obj->name = vpip_name_string("W<>");
      obj->subtype = vpiRealConst;
      assert(base < 65536);
      obj->index = base;

      return &obj->base;
}

#ifdef CHECK_WITH_VALGRIND
void thread_word_delete(vpiHandle item)
{
      handle_map[item] = false;
}

static void thread_word_delete_real(vpiHandle item)
{
      struct __vpiVThrWord*obj = (struct __vpiVThrWord*)item;
      free(obj);
}

void vpi_handle_delete()
{
      map<vpiHandle, bool>::iterator iter;
      for (iter = handle_map.begin(); iter != handle_map.end(); iter++) {
	    if (iter->second) thread_vthr_delete_real(iter->first);
	    else thread_word_delete_real(iter->first);
      }
}
#endif
