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
#ident "$Id: vpi_signal.cc,v 1.19 2001/07/13 03:02:34 steve Exp $"
#endif

/*
 * vpiReg handles are handled here. These objects represent vectors of
 * .var objects that can be manipulated by the VPI module.
 */

# include  "vpi_priv.h"
# include  "functor.h"
# include  <stdio.h>
# include  <malloc.h>
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

	  default:
	    return 0;
      }
}

static char* signal_get_str(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      struct __vpiSignal*rfp = (struct __vpiSignal*)ref;

      static char full_name[4096];

      switch (code) {

	  case vpiFullName:
	    strcpy(full_name, vpi_get_str(vpiFullName, &rfp->scope->base));
	    strcat(full_name, ".");
	    strcat(full_name, rfp->name);
	    return full_name;

	  case vpiName:
	    return (char*)rfp->name;
      }

      return 0;
}

static char buf[4096];

static void signal_vpiDecStrVal(struct __vpiSignal*rfp, s_vpi_value*vp)
{
      unsigned wid = (rfp->msb >= rfp->lsb)
	    ? (rfp->msb - rfp->lsb + 1)
	    : (rfp->lsb - rfp->msb + 1);

      unsigned long val = 0;
      unsigned count_x = 0, count_z = 0;

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_ipoint_t fptr = ipoint_index(rfp->bits, wid-idx-1);
	    val *= 2;
	    switch (functor_oval(fptr)) {
		case 0:
		  break;
		case 1:
		  val += 1;
		  break;
		case 2:
		  count_x += 1;
		  break;
		case 3:
		  count_z += 1;
		  break;
	    }
      }

      if (count_x == wid) {
	    buf[0] = 'x';
	    buf[1] = 0;
	    return;
      }

      if (count_x > 0) {
	    buf[0] = 'X';
	    buf[1] = 0;
	    return;
      }

      if (count_z == wid) {
	    buf[0] = 'z';
	    buf[1] = 0;
	    return;
      }

      if (count_z > 0) {
	    buf[0] = 'Z';
	    buf[1] = 0;
	    return;
      }

      if (rfp->signed_flag) {
	    long tmp;
            assert(sizeof(tmp) == sizeof(val));
	    if (val & (1<<(wid-1))  &&  wid < 8*sizeof(tmp)) {
		  tmp = -1;
		  tmp <<= wid;
		  tmp |= val;
	    } else {
		  tmp = val;
	    }
            sprintf(buf, "%ld", tmp);

      } else {
	    sprintf(buf, "%lu", val);
      }
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

      switch (vp->format) {

	  case vpiIntVal:
	    assert(wid <= 8 * sizeof vp->value.integer);
	    vp->value.integer = 0;
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  vvp_ipoint_t fptr = ipoint_index(rfp->bits, idx);
		  switch (functor_oval(fptr)) {
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
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  vvp_ipoint_t fptr = ipoint_index(rfp->bits, idx);
		  buf[wid-idx-1] = "01xz"[functor_oval(fptr)];
	    }
	    buf[wid] = 0;
	    vp->value.str = buf;
	    break;

	  case vpiHexStrVal: {
		unsigned hval, hwid;
		hwid = (wid + 3) / 4;
		buf[hwid] = 0;
		hval = 0;
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      vvp_ipoint_t fptr = ipoint_index(rfp->bits, idx);
		      hval = hval | (functor_oval(fptr) << 2*(idx % 4));

		      if (idx%4 == 3) {
			    hwid -= 1;
			    buf[hwid] = hex_digits[hval];
			    hval = 0;
		      }
		}

		if (hwid > 0) {
		      hwid -= 1;
		      buf[hwid] = hex_digits[hval];
		      hval = 0;
		}
		vp->value.str = buf;
		break;
	  }

	  case vpiOctStrVal: {
		unsigned hval, hwid;
		hwid = (wid + 2) / 3;
		buf[hwid] = 0;
		hval = 0;
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      vvp_ipoint_t fptr = ipoint_index(rfp->bits, idx);
		      hval = hval | (functor_oval(fptr) << 2*(idx % 3));

		      if (idx%3 == 2) {
			    hwid -= 1;
			    buf[hwid] = oct_digits[hval];
			    hval = 0;
		      }
		}

		if (hwid > 0) {
		      hwid -= 1;
		      buf[hwid] = oct_digits[hval];
		      hval = 0;
		}
		vp->value.str = buf;
		break;
	  }

	  case vpiDecStrVal:
	    signal_vpiDecStrVal(rfp, vp);
	    vp->value.str = buf;
	    break;

	  default:
	      /* XXXX Not implemented yet. */
	    assert(0);
      }
}

/*
 * The put_value method writes the value into the vector, and returns
 * the affected ref. This operation works much like the %set or
 * %assign instructions and causes all the side-effects that the
 * equivilent instruction would cause.
 */
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
		      functor_set(ipoint_index(rfp->bits,idx), val&1,
				  (val&1)? St1 : St0, true);
		      val >>= 1;
		}
		break;
	  }

	  case vpiScalarVal:
	    switch (vp->value.scalar) {
		case vpi0:
		  functor_set(rfp->bits, 0, St0, true);
		  break;
		case vpi1:
		  functor_set(rfp->bits, 1, St1, true);
		  break;
		case vpiX:
		  functor_set(rfp->bits, 2, StX, true);
		  break;
		case vpiZ:
		  functor_set(rfp->bits, 3, HiZ, true);
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
		      int bit = (aval&1) | ((bval<<1)&2);
		      switch (bit) {
			  case 0: /* zero */
			    functor_set(ipoint_index(rfp->bits,idx),
					0, St0, true);
			    break;
			  case 1: /* one */
			    functor_set(ipoint_index(rfp->bits,idx),
					1, St1, true);
			    break;
			  case 2: /* z */
			    functor_set(ipoint_index(rfp->bits,idx),
					3, HiZ, true);
			    break;
			  case 3: /* x */
			    functor_set(ipoint_index(rfp->bits,idx),
					2, StX, true);
			    break;
		      }
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

struct __vpiSignal* vpip_sig_from_ptr(vvp_ipoint_t ptr)
{
      functor_t fu = functor_index(ptr);
      return fu->sig;
}

static const struct __vpirt vpip_reg_rt = {
      vpiReg,
      signal_get,
      signal_get_str,
      signal_get_value,
      signal_put_value,
      0,
      0
};

static const struct __vpirt vpip_net_rt = {
      vpiNet,
      signal_get,
      signal_get_str,
      signal_get_value,
      signal_put_value,
      0,
      0
};

/*
 * Construct a vpiReg object. It's like a net, except for the type.
 */
vpiHandle vpip_make_reg(char*name, int msb, int lsb, bool signed_flag,
			vvp_ipoint_t base)
{
      vpiHandle obj = vpip_make_net(name, msb,lsb, signed_flag, base);
      obj->vpi_type = &vpip_reg_rt;
      return obj;
}

/*
 * Construct a vpiNet object. Give the object specified dimensions,
 * and point to the specified functor for the lsb.
 */
vpiHandle vpip_make_net(char*name, int msb, int lsb, bool signed_flag,
			vvp_ipoint_t base)
{
      struct __vpiSignal*obj = (struct __vpiSignal*)
	    malloc(sizeof(struct __vpiSignal));
      obj->base.vpi_type = &vpip_net_rt;
      obj->name = name;
      obj->msb = msb;
      obj->lsb = lsb;
      obj->signed_flag = signed_flag? 1 : 0;
      obj->bits = base;
      obj->callbacks = 0;

      obj->scope = vpip_peek_current_scope();

      unsigned wid = (obj->msb > obj->lsb)
	    ? obj->msb - obj->lsb + 1
	    : obj->lsb - obj->msb + 1;
      for (unsigned i=0; i<wid; i++) {
	    vvp_ipoint_t ii = ipoint_index(obj->bits, i);
	    functor_t fu = functor_index(ii);
	    fu->sig = obj;
      }
	    
      return &obj->base;
}


/*
 * $Log: vpi_signal.cc,v $
 * Revision 1.19  2001/07/13 03:02:34  steve
 *  Rewire signal callback support for fast lookup. (Stephan Boettcher)
 *
 * Revision 1.18  2001/06/29 00:44:56  steve
 *  Properly support signal full names.
 *
 * Revision 1.17  2001/06/21 23:05:08  steve
 *  Some documentation of callback behavior.
 *
 * Revision 1.16  2001/06/21 22:54:12  steve
 *  Support cbValueChange callbacks.
 *
 * Revision 1.15  2001/05/30 03:02:35  steve
 *  Propagate strength-values instead of drive strengths.
 *
 * Revision 1.14  2001/05/22 04:08:49  steve
 *  correctly interpret signed decimal values.
 *
 * Revision 1.13  2001/05/15 15:09:08  steve
 *  Add the glossary file.
 *
 * Revision 1.12  2001/05/14 00:42:32  steve
 *  test width of target with bit size of long.
 *
 * Revision 1.11  2001/05/09 04:23:19  steve
 *  Now that the interactive debugger exists,
 *  there is no use for the output dump.
 *
 * Revision 1.10  2001/05/08 23:32:26  steve
 *  Add to the debugger the ability to view and
 *  break on functors.
 *
 *  Add strengths to functors at compile time,
 *  and Make functors pass their strengths as they
 *  propagate their output.
 */

