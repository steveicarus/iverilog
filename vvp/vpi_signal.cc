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
#ident "$Id: vpi_signal.cc,v 1.52 2003/02/09 23:33:26 steve Exp $"
#endif

/*
 * vpiReg handles are handled here. These objects represent vectors of
 * .var objects that can be manipulated by the VPI module.
 */

# include  "vpi_priv.h"
# include  "functor.h"
# include  "statistics.h"
# include  <stdio.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

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
 * The string values need a result buf to hold the results. This
 * buffer can be reused for that purpose. Whenever I have a need, the
 * need_result_buf function makes sure that need can be met.
 */
char *need_result_buf(unsigned cnt, vpi_rbuf_t type)
{
      cnt = (cnt + 0x0fff) & ~0x0fff;

      static char*result_buf[2] = {0, 0};
      static size_t result_buf_size[2] = {0, 0};

      if (result_buf_size[type] == 0) {
	    result_buf[type] = (char*)malloc(cnt);
	    result_buf_size[type] = cnt;
      } else if (result_buf_size[type] < cnt) {
	    result_buf[type] = (char*)realloc(result_buf[type], cnt);
	    result_buf_size[type] = cnt;
      }

      return result_buf[type];
}

/*
 * implement vpi_get for vpiReg objects.
 */
static int signal_get(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;

      switch (code) {

	  case vpiSigned:
	    return rfp->signed_flag != 0;

	  case vpiSize:
	    if (rfp->msb >= rfp->lsb)
		  return rfp->msb - rfp->lsb + 1;
	    else
		  return rfp->lsb - rfp->msb + 1;

	  case _vpiNexusId:
	    if (rfp->msb == rfp->lsb)
		  return vvp_fvector_get(rfp->bits, 0);
	    else
		  return 0;

	  default:
	    return 0;
      }
}

static char* signal_get_str(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;

      char *bn = strdup(vpi_get_str(vpiFullName, &rfp->scope->base));
      char *nm = (char*)rfp->name;

      char *rbuf = need_result_buf(strlen(bn) + strlen(nm) + 2, RBUF_STR);

      switch (code) {

	  case vpiFullName:
	    sprintf(rbuf, "%s.%s", bn, nm);
	    free(bn);
	    return rbuf;

	  case vpiName:
	    strcpy(rbuf, nm);
	    free(bn);
	    return rbuf;
      }

      free(bn);
      return 0;
}

static vpiHandle signal_get_handle(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;

      switch (code) {

	  case vpiScope:
	    return &rfp->scope->base;
      }

      return 0;
}


static char *signal_vpiDecStrVal(struct __vpiSignal*rfp, s_vpi_value*vp)
{
      unsigned wid = (rfp->msb >= rfp->lsb)
	    ? (rfp->msb - rfp->lsb + 1)
	    : (rfp->lsb - rfp->msb + 1);

      unsigned char*bits = new unsigned char[wid];
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_ipoint_t fptr = vvp_fvector_get(rfp->bits, idx);
	    bits[idx] = functor_get(fptr);
      }

      unsigned hwid = (wid+2) / 3 + 1;
      char *rbuf = need_result_buf(hwid, RBUF_VAL);

      vpip_bits_to_dec_str(bits, wid, rbuf, hwid, rfp->signed_flag);

      delete[]bits;

      return rbuf;
}


static char *signal_vpiStringVal(struct __vpiSignal*rfp, s_vpi_value*vp)
{
      unsigned wid = (rfp->msb >= rfp->lsb)
	    ? (rfp->msb - rfp->lsb + 1)
	    : (rfp->lsb - rfp->msb + 1);


      /* The result will use a character for each 8 bits of the
	 vector. Add one extra character for the highest bits that
	 don't form an 8 bit group. */
      char *rbuf = need_result_buf(wid/8 + ((wid&7)!=0) + 1, RBUF_VAL);
      char *cp = rbuf;

      char tmp = 0;
      int bitnr;
      for(bitnr=wid-1; bitnr>=0; bitnr--){
	  vvp_ipoint_t fptr = vvp_fvector_get(rfp->bits, bitnr);
	  tmp <<= 1;

	  switch (functor_get(fptr)) {
	  case 0:
	      break;
	  case 1:
	      tmp |= 1;
	      break;
	  default:
	      break;
	  }

	  if ((bitnr&7)==0){
	      *cp++ = tmp? tmp : ' ';
	      tmp = 0;
	  }
      }
      *cp++ = 0;

      return rbuf;
}

/*
 * The get_value method reads the values of the functors and returns
 * the vector to the caller. This causes no side-effect, and reads the
 * variables like a %load would.
 */
static void signal_get_value(vpiHandle ref, s_vpi_value*vp)
{
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;

      unsigned wid = (rfp->msb >= rfp->lsb)
	    ? (rfp->msb - rfp->lsb + 1)
	    : (rfp->lsb - rfp->msb + 1);
      char *rbuf = 0;

      switch (vp->format) {

	  case vpiIntVal:
	    assert(wid <= 8 * sizeof vp->value.integer);
	    vp->value.integer = 0;
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  vvp_ipoint_t fptr = vvp_fvector_get(rfp->bits, idx);
		  switch (functor_get(fptr)) {
		      case 0:
			break;
		      case 1:
			vp->value.integer |= 1<<idx;
			break;
		      default:
			idx = wid;
			vp->value.integer = 0;
			break;
		  }
	    }
	    break;

	  case vpiBinStrVal:
	    rbuf = need_result_buf(wid+1, RBUF_VAL);

	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  vvp_ipoint_t fptr = vvp_fvector_get(rfp->bits, idx);
		  rbuf[wid-idx-1] = "01xz"[functor_get(fptr)];
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
		      vvp_ipoint_t fptr = vvp_fvector_get(rfp->bits, idx);
		      hval = hval | (functor_get(fptr) << 2*(idx % 4));

		      if (idx%4 == 3) {
			    hwid -= 1;
			    rbuf[hwid] = hex_digits[hval];
			    hval = 0;
		      }
		}

		if (hwid > 0) {
		      hwid -= 1;
		      rbuf[hwid] = hex_digits[hval];
		      unsigned padd = 0;
		      switch(rbuf[hwid]) {
			  case 'X': padd = 2; break;
			  case 'Z': padd = 3; break;
		      }
		      if (padd) {
			    for (unsigned idx = wid % 4; idx < 4; idx += 1) {
				  hval = hval | padd << 2*idx;
			    }
			    rbuf[hwid] = hex_digits[hval];
		      }
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
		      vvp_ipoint_t fptr = vvp_fvector_get(rfp->bits, idx);
		      hval = hval | (functor_get(fptr) << 2*(idx % 3));

		      if (idx%3 == 2) {
			    hwid -= 1;
			    rbuf[hwid] = oct_digits[hval];
			    hval = 0;
		      }
		}

		if (hwid > 0) {
		      hwid -= 1;
		      rbuf[hwid] = oct_digits[hval];
		      unsigned padd = 0;
		      switch(rbuf[hwid]) {
			  case 'X': padd = 2; break;
			  case 'Z': padd = 3; break;
		      }
		      if (padd) {
			    for (unsigned idx = wid % 3; idx < 3; idx += 1) {
				  hval = hval | padd << 2*idx;
			    }
			    rbuf[hwid] = oct_digits[hval];
		      }
		}
		vp->value.str = rbuf;
		break;
	  }

	  case vpiDecStrVal:
	    vp->value.str = signal_vpiDecStrVal(rfp, vp);
	    break;

	  case vpiStringVal:
	    vp->value.str = signal_vpiStringVal(rfp, vp);
	    break;

	  case vpiVectorVal: {
	      unsigned int obit = 0;
	      unsigned hwid = (wid - 1)/32 + 1;

	      rbuf = need_result_buf(hwid * sizeof(s_vpi_vecval), RBUF_VAL);
	      s_vpi_vecval *op = (p_vpi_vecval)rbuf;
	      vp->value.vector = op;

	      op->aval = op->bval = 0;
	      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		vvp_ipoint_t fptr = vvp_fvector_get(rfp->bits, idx);
		switch (functor_get(fptr)) {
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
		      if ((op - vp->value.vector) < (ptrdiff_t)hwid)
			    op->aval = op->bval = 0;
		      obit = 0;
		}
	      }
	      break;
	    }

	  case vpiStrengthVal: {
		s_vpi_strengthval*op = (s_vpi_strengthval*)
		      need_result_buf(wid * sizeof(s_vpi_strengthval),
				      RBUF_VAL);
		vp->value.strength = op;

		  /* Convert the internal strength values of each
		     functor in the vector to a PLI2.0 version. */
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      vvp_ipoint_t fptr = vvp_fvector_get(rfp->bits, idx);
		      functor_t fp = functor_index(fptr);

		      unsigned str = fp->get_ostr();
		      unsigned s0 = 1 << (str&0x07);
		      unsigned s1 = 1 << ((str>>4) & 0x07);

		      switch (fp->get()) {
			  case 0:
			    op[idx].logic = vpi0;
			    op[idx].s0 = s0|s1;
			    op[idx].s1 = 0;
			    break;
			  case 1:
			    op[idx].logic = vpi1;
			    op[idx].s0 = 0;
			    op[idx].s1 = s0|s1;
			    break;
			  case 2:
			    op[idx].logic = vpiX;
			    op[idx].s0 = s0;
			    op[idx].s1 = s1;
			    break;
			  case 3:
			    op[idx].logic = vpiZ;
			    op[idx].s0 = vpiHiZ;
			    op[idx].s1 = vpiHiZ;
			    break;
			  default:
			    assert(0);
		      }
		}

		break;
	  }

	  default:
	    fprintf(stderr, "vvp internal error: get_value: "
		    "value type %u not implemented."
		    " Signal is %s in scope %s\n",
		    vp->format, rfp->name, rfp->scope->name);
	    assert(0);
      }
}

/*
 * The put_value method writes the value into the vector, and returns
 * the affected ref. This operation works much like the %set or
 * %assign instructions and causes all the side-effects that the
 * equivalent instruction would cause.
 */

static void functor_poke(struct __vpiSignal*rfp, unsigned idx,
			 unsigned val, unsigned str)
{
      vvp_ipoint_t ptr = vvp_fvector_get(rfp->bits,idx);
      functor_t fu = functor_index(ptr);
      fu->put_ostr(val, str, true);
}

static void signal_put_stringval(struct __vpiSignal*rfp, unsigned wid,
				 const char*str)
{
      unsigned idx;
      const char*cp;

      cp = str + strlen(str);
      idx = 0;

      while ((idx < wid) && (cp > str)) {
	    unsigned byte = *--cp;
	    int bit;

	    for (bit = 0 ;  bit < 8 ;  bit += 1) {
		  if (byte & 1)
			functor_poke(rfp, idx, 1, St1);
		  else
			functor_poke(rfp, idx, 0, St0);

		  byte >>= 1;
		  idx += 1;
	    }
      }

      while (idx < wid) {
	    functor_poke(rfp, 0, 0, St0);
	    idx += 1;
      }
}

static vpiHandle signal_put_value(vpiHandle ref, s_vpi_value*vp,
				  p_vpi_time when, int flags)
{
      unsigned wid;
      struct __vpiSignal*rfp;

      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      rfp = (struct __vpiSignal*)ref;

	/* XXXX delays are not yet supported. */
      assert(flags == vpiNoDelay);

      wid = (rfp->msb >= rfp->lsb)
	    ? (rfp->msb - rfp->lsb + 1)
	    : (rfp->lsb - rfp->msb + 1);

      switch (vp->format) {

	  case vpiIntVal: {
		if (wid > 8*sizeof(long)) {
		      fprintf(stderr, "internal error: wid(%u) "
			      "too large.\n", wid);
		      assert(0);
		}

		long val = vp->value.integer;
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      functor_poke(rfp, idx, val&1, (val&1)? St1 : St0);
		      val >>= 1;
		}
		break;
	  }

	  case vpiScalarVal:
	    switch (vp->value.scalar) {
		case vpi0:
		  functor_poke(rfp, 0, 0, St0);
		  break;
		case vpi1:
		  functor_poke(rfp, 0, 1, St1);
		  break;
		case vpiX:
		  functor_poke(rfp, 0, 2, StX);
		  break;
		case vpiZ:
		  functor_poke(rfp, 0, 3, HiZ);
		  break;
		default:
		  assert(0);
	    }
	    break;

	  case vpiVectorVal:
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  unsigned long aval = vp->value.vector[idx/32].aval;
		  unsigned long bval = vp->value.vector[idx/32].bval;
		  aval >>= idx%32;
		  bval >>= idx%32;
		  int bit = (aval&1) | ((bval<<1)&2);
		  switch (bit) {
		      case 0: /* zero */
			functor_poke(rfp,idx, 0, St0);
			break;
		      case 1: /* one */
			functor_poke(rfp,idx, 1, St1);
			break;
		      case 2: /* z */
			functor_poke(rfp,idx, 3, HiZ);
			break;
		      case 3: /* x */
			functor_poke(rfp,idx, 2, StX);
			break;
		  }
	    }
	    break;

	  case vpiBinStrVal: {
		unsigned char*bits = new unsigned char[(wid+3) / 4];
		vpip_bin_str_to_bits(bits, wid, vp->value.str, false);

		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      unsigned bb = idx / 4;
		      unsigned bs = (idx % 4) * 2;
		      unsigned val = (bits[bb] >> bs) & 0x03;

		      switch (val) {
			  case 0: /* zero */
			    functor_poke(rfp,idx, 0, St0);
			    break;
			  case 1: /* one */
			    functor_poke(rfp,idx, 1, St1);
			    break;
			  case 2: /* x */
			    functor_poke(rfp,idx, 2, StX);
			    break;
			  case 3: /* z */
			    functor_poke(rfp,idx, 3, HiZ);
			    break;
		      }
		}

		delete[]bits;
		break;
	  }

	  case vpiOctStrVal: {
		unsigned char*bits = new unsigned char[(wid+3) / 4];
		vpip_oct_str_to_bits(bits, wid, vp->value.str, false);

		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      unsigned bb = idx / 4;
		      unsigned bs = (idx % 4) * 2;
		      unsigned val = (bits[bb] >> bs) & 0x03;

		      switch (val) {
			  case 0: /* zero */
			    functor_poke(rfp,idx, 0, St0);
			    break;
			  case 1: /* one */
			    functor_poke(rfp,idx, 1, St1);
			    break;
			  case 2: /* x */
			    functor_poke(rfp,idx, 2, StX);
			    break;
			  case 3: /* z */
			    functor_poke(rfp,idx, 3, HiZ);
			    break;
		      }
		}

		delete[]bits;
		break;
	  }

	  case vpiHexStrVal: {
		unsigned char*bits = new unsigned char[(wid+3) / 4];
		vpip_hex_str_to_bits(bits, wid, vp->value.str, false);

		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      unsigned bb = idx / 4;
		      unsigned bs = (idx % 4) * 2;
		      unsigned val = (bits[bb] >> bs) & 0x03;

		      switch (val) {
			  case 0: /* zero */
			    functor_poke(rfp,idx, 0, St0);
			    break;
			  case 1: /* one */
			    functor_poke(rfp,idx, 1, St1);
			    break;
			  case 2: /* x */
			    functor_poke(rfp,idx, 2, StX);
			    break;
			  case 3: /* z */
			    functor_poke(rfp,idx, 3, HiZ);
			    break;
		      }
		}

		delete[]bits;
		break;
	  }

	  case vpiDecStrVal: {
		unsigned char*bits = new unsigned char[wid];
		vpip_dec_str_to_bits(bits, wid, vp->value.str, false);

		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {

		      switch (bits[idx]) {
			  case 0: /* zero */
			    functor_poke(rfp,idx, 0, St0);
			    break;
			  case 1: /* one */
			    functor_poke(rfp,idx, 1, St1);
			    break;
			  case 2: /* x */
			    functor_poke(rfp,idx, 2, StX);
			    break;
			  case 3: /* z */
			    functor_poke(rfp,idx, 3, HiZ);
			    break;
		      }
		}

		delete[]bits;
		break;
	  }

	  case vpiStringVal:
	    signal_put_stringval(rfp, wid, vp->value.str);
	    break;

	  default:
	    fprintf(stderr, "vvp internal error: put_value: "
		    "value type %u not implemented."
		    " Signal is %s in scope %s\n",
		    vp->format, rfp->name, rfp->scope->name);
	    assert(0);

      }

      return ref;
}

static const struct __vpirt vpip_reg_rt = {
      vpiReg,
      signal_get,
      signal_get_str,
      signal_get_value,
      signal_put_value,
      signal_get_handle,
      0
};

static const struct __vpirt vpip_net_rt = {
      vpiNet,
      signal_get,
      signal_get_str,
      signal_get_value,
      signal_put_value,
      signal_get_handle,
      0
};

/*
 * Construct a vpiIntegetVar object. Indicate the type using a flag
 * to minimize the code modifications. Icarus implements integers
 * as 'reg signed [31:0]'.
 */
vpiHandle vpip_make_int(const char*name, int msb, int lsb, vvp_fvector_t vec)
{
      vpiHandle obj = vpip_make_net(name, msb,lsb, true, vec);
      struct __vpiSignal*rfp = (struct __vpiSignal*)obj;
      obj->vpi_type = &vpip_reg_rt;
      rfp->isint_ = true;
      return obj;
}

/*
 * Construct a vpiReg object. It's like a net, except for the type.
 */
vpiHandle vpip_make_reg(const char*name, int msb, int lsb,
			bool signed_flag, vvp_fvector_t vec)
{
      vpiHandle obj = vpip_make_net(name, msb,lsb, signed_flag, vec);
      obj->vpi_type = &vpip_reg_rt;
      return obj;
}

/*
 * Construct a vpiNet object. Give the object specified dimensions,
 * and point to the specified functor for the lsb.
 */
vpiHandle vpip_make_net(const char*name, int msb, int lsb,
			bool signed_flag, vvp_fvector_t vec)
{
      struct __vpiSignal*obj = (struct __vpiSignal*)
	    malloc(sizeof(struct __vpiSignal));
      obj->base.vpi_type = &vpip_net_rt;
      obj->name = vpip_string(name);
      obj->msb = msb;
      obj->lsb = lsb;
      obj->signed_flag = signed_flag? 1 : 0;
      obj->isint_ = false;
      obj->bits = vec;
      obj->callback = 0;

      obj->scope = vpip_peek_current_scope();

      count_vpi_nets += 1;

      return &obj->base;
}


/*
 * $Log: vpi_signal.cc,v $
 * Revision 1.52  2003/02/09 23:33:26  steve
 *  Spelling fixes.
 *
 * Revision 1.51  2003/01/07 18:07:50  steve
 *  Allocate res-buf in bigger chunks
 *
 * Revision 1.50  2002/11/25 23:33:45  steve
 *  Support put of vpiStringVal to signals.
 *
 * Revision 1.49  2002/09/11 16:06:57  steve
 *  Fix wrecked rbuf in vpi_get_str of signals and memories.
 *
 * Revision 1.48  2002/09/10 02:27:11  steve
 *  Actually set strength pointer when getting strength val.
 *
 * Revision 1.47  2002/09/06 04:56:29  steve
 *  Add support for %v is the display system task.
 *  Change the encoding of H and L outputs from
 *  the bufif devices so that they are logic x.
 *
 * Revision 1.46  2002/08/12 01:35:09  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.45  2002/07/23 15:11:41  steve
 *  integral type/ptrdiff_t warning.
 *
 * Revision 1.44  2002/07/23 02:36:34  steve
 *  Careful not to overrun vector buffer.
 *
 * Revision 1.43  2002/07/19 00:36:36  steve
 *  Support put of wide vpiVectorVal to signal.
 *
 * Revision 1.42  2002/07/09 03:24:37  steve
 *  Dynamic resizevpi result buf in more places.
 *
 * Revision 1.41  2002/07/05 17:14:15  steve
 *  Names of vpi objects allocated as vpip_strings.
 *
 * Revision 1.40  2002/07/05 02:50:58  steve
 *  Remove the vpi object symbol table after compile.
 *
 * Revision 1.39  2002/07/03 23:39:57  steve
 *  Dynamic size result buffer for _str and _get_value functions.
 *
 * Revision 1.38  2002/07/03 23:16:27  steve
 *  don't pollute name space
 *  fix vecval for Z/X cases
 *
 * Revision 1.37  2002/07/03 02:09:38  steve
 *  vpiName, vpiFullName support in memory types,
 *  length checks for *_get_str() buffers,
 *  temporary buffers for *_get_str() data,
 *  dynamic storage for vpi_get_data() in memory types
 *  shared with signal white space
 *
 * Revision 1.36  2002/06/30 02:52:36  steve
 *  vpiVectorVal of very wide signals.
 *
 * Revision 1.35  2002/06/21 04:58:55  steve
 *  Add support for special integer vectors.
 *
 * Revision 1.34  2002/05/15 04:48:46  steve
 *  Support set by string for reg objects.
 *
 * Revision 1.33  2002/02/03 01:01:51  steve
 *  Use Larrys bits-to-decimal-string code.
 *
 * Revision 1.32  2002/01/09 03:29:12  steve
 *  String prints of non-round vectors (PR378)
 *
 * Revision 1.31  2001/12/18 05:31:54  steve
 *  Remove result length restrictions for vpi_get_value.
 *
 * Revision 1.30  2001/12/06 03:31:25  steve
 *  Support functor delays for gates and UDP devices.
 *  (Stephan Boettcher)
 *
 * Revision 1.29  2001/11/07 03:34:42  steve
 *  Use functor pointers where vvp_ipoint_t is unneeded.
 *
 * Revision 1.28  2001/10/31 04:27:47  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.27  2001/10/18 04:52:31  steve
 *  Support vpiVectorVal for signals. (Philip Blundell)
 *
 * Revision 1.26  2001/10/15 01:49:50  steve
 *  Support getting scope of scope, and scope of signals.
 *
 * Revision 1.25  2001/09/30 05:18:46  steve
 *  Reduce VCD output by removing duplicates. (Stephan Boettcher)
 *
 * Revision 1.24  2001/09/15 18:27:05  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.23  2001/08/09 19:38:23  steve
 *  Nets (wires) do not use their own functors.
 *  Modifications to propagation of values.
 *  (Stephan Boettcher)
 */
