/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: vpi_vthr_vector.cc,v 1.4 2001/12/30 21:31:38 steve Exp $"
#endif

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
      unsigned short bas;
      unsigned short wid;
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
	    return 0;

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

static char buf[4096];

static void vthr_vec_DecStrVal(struct __vpiVThrVec*rfp, s_vpi_value*vp)
{
      unsigned long val = 0;
      unsigned count_x = 0, count_z = 0;

      for (unsigned idx = 0 ;  idx < rfp->wid ;  idx += 1) {
	    val *= 2;
	    switch (get_bit(rfp, rfp->wid-idx-1)) {
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

      if (count_x == rfp->wid) {
	    buf[0] = 'x';
	    buf[1] = 0;
	    return;
      }

      if (count_x > 0) {
	    buf[0] = 'X';
	    buf[1] = 0;
	    return;
      }

      if (count_z == rfp->wid) {
	    buf[0] = 'z';
	    buf[1] = 0;
	    return;
      }

      if (count_z > 0) {
	    buf[0] = 'Z';
	    buf[1] = 0;
	    return;
      }

      sprintf(buf, "%lu", val);
}

static void vthr_vec_StringVal(struct __vpiVThrVec*rfp, s_vpi_value*vp)
{
      assert(rfp->wid % 8 == 0);
      assert(rfp->wid/8 < sizeof buf);

      unsigned bytes = rfp->wid/8;

      for (unsigned idx = 0 ;  idx < bytes ;  idx += 1) {
	    unsigned base = rfp->wid - 8 - idx * 8;

	    int val = 0;
	    for (unsigned bit = 0 ;  bit < 8 ;  bit += 1) {
		  unsigned tmp = get_bit(rfp, base+bit);
		  if (tmp == 1)
			val |= 1 << bit;
	    }

	    buf[idx] = val? val : ' ';
      }

      buf[bytes] = 0;
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
      
      unsigned wid = rfp->wid;

      switch (vp->format) {

	  case vpiBinStrVal:
	    assert(wid < sizeof buf);
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  buf[wid-idx-1] = "01xz"[get_bit(rfp, idx)];
	    }
	    buf[wid] = 0;
	    vp->value.str = buf;
	    break;
	    
	  case vpiHexStrVal: {
		unsigned hval, hwid;
		hwid = (wid + 3) / 4;
		assert(hwid < sizeof buf);
		buf[hwid] = 0;
		hval = 0;
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      hval = hval | (get_bit(rfp, idx) << 2*(idx % 4));

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
		assert(hwid < sizeof buf);
		buf[hwid] = 0;
		hval = 0;
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		      hval = hval | (get_bit(rfp,idx) << 2*(idx % 3));

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
	    vthr_vec_DecStrVal(rfp, vp);
	    vp->value.str = buf;
	    break;

	  case vpiStringVal:
	    vthr_vec_StringVal(rfp, vp);
	    vp->value.str = buf;
	    break;

	  default:
	      /* XXXX Not implemented yet. */
	    assert(0);
      }
}

/*
 * The put_value method writes the value into the vector.
 */
static vpiHandle vthr_vec_put_value(vpiHandle ref, s_vpi_value*vp,
				  p_vpi_time when, int flags)
{
      assert((ref->vpi_type->type_code==vpiNet)
	     || (ref->vpi_type->type_code==vpiReg));

      struct __vpiVThrVec*rfp = (struct __vpiVThrVec*)ref;

	/* XXXX delays are not yet supported. */
      assert(flags == vpiNoDelay);

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
// create such things, yet.  Lacking a neme, for example.

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
vpiHandle vpip_make_vthr_vector(unsigned base, unsigned wid)
{
      struct __vpiVThrVec*obj = (struct __vpiVThrVec*)
	    malloc(sizeof(struct __vpiVThrVec));
      obj->base.vpi_type = &vpip_vthr_const_rt;
      obj->bas = base;
      obj->wid = wid;
      obj->name = "T<>";

      return &obj->base;
}


/*
 * $Log: vpi_vthr_vector.cc,v $
 * Revision 1.4  2001/12/30 21:31:38  steve
 *  Support vpiStringVal in vhtread vectors.
 *
 * Revision 1.3  2001/09/15 18:27:05  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.2  2001/05/20 00:40:12  steve
 *  Get bit ordering right when making decimal strings.
 *
 * Revision 1.1  2001/05/10 00:26:53  steve
 *  VVP support for memories in expressions,
 *  including general support for thread bit
 *  vectors as system task parameters.
 *  (Stephan Boettcher)
 *
 */

