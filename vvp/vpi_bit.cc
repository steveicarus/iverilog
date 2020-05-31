/*
 * Copyright (C) 2020 Cary R. (cygcary@yahoo.com)
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

/*
 * vpiNetBit and vpiRegBit are handled here along with some help from their
 * parent singal or net.
 */

# include "compile.h"
# include "vpi_priv.h"
# include <cassert>


static const __vpiBit*bit_from_handle(const __vpiHandle*ref)
{
      if (ref == NULL) return NULL;
      const __vpiBit::as_bit_t*ptr = dynamic_cast<const __vpiBit::as_bit_t*> (ref);
      if (ptr == NULL) return NULL;
      return (const struct __vpiBit*) ref;
}

static __vpiBit*bit_from_handle(__vpiHandle*ref)
{
      if (ref == NULL) return NULL;
      __vpiBit::as_bit_t*ptr = dynamic_cast<__vpiBit::as_bit_t*> (ref);
      if (ptr == NULL) return NULL;
      return (struct __vpiBit*) ref;
}

static int bit_get_type(const __vpiBit*rfp)
{
      assert(rfp);

      const struct __vpiSignal*parent = rfp->get_parent();
      assert(parent);

      switch (parent->get_type_code()) {
	case vpiNet:
	    return vpiNetBit;
	case vpiReg:
	    return vpiRegBit;
      }
      assert(0);
}

static int bit_get(int code, vpiHandle ref)
{
      struct __vpiBit*rfp = bit_from_handle(ref);
      assert(rfp);

      struct __vpiSignal*parent = rfp->get_parent();
      assert(parent);

      switch (code) {
	case vpiArray:  // A bit is not an array
	    return 0;

	case vpiAutomatic:
	    return vpi_get(vpiAutomatic, parent);

	case vpiIndex:
	    {
		  s_vpi_value vp;
		  vp.format = vpiIntVal;
		  vpi_get_value(rfp->index, &vp);
		  return vp.value.integer;
	    }

	case vpiLineNo:
	    return vpi_get(vpiLineNo, parent);

	case vpiScalar:  // A bit is a scalar
	    return 1;

	case vpiSigned:  // A bit is unsigned
	    return 0;

	case vpiSize:  // A bit has a width of 1
	    return 1;

	case vpiVector:  // A bit is not a vector
	    return 0;

	default:
	    fprintf(stderr, "VPI error: unknown bit_get property %d.\n",
	            code);
	    return vpiUndefined;
      }
}

static char* bit_get_str(int code, vpiHandle ref)
{
      struct __vpiBit*rfp = bit_from_handle(ref);
      assert(rfp);

      if (code == vpiFile) {  // Not implemented for now!
            return simple_set_rbuf_str(file_names[0]);
      }

      if ((code != vpiName) && (code != vpiFullName)) return NULL;

      struct __vpiSignal*parent = rfp->get_parent();
      assert(parent);

      char *nm, *ixs;
      nm = strdup(vpi_get_str(vpiName, parent));
      s_vpi_value vp;
      vp.format = vpiDecStrVal;
      vpi_get_value(rfp->index, &vp);
      ixs = vp.value.str;

      char *rbuf = generic_get_str(code, vpip_scope(parent), nm, ixs);
      free(nm);
      return rbuf;
}

static vpiHandle bit_get_handle(int code, vpiHandle ref)
{
      struct __vpiBit*rfp = bit_from_handle(ref);
      assert(rfp);

      struct __vpiSignal*parent = rfp->get_parent();
      assert(parent);

      switch (code) {
	case vpiIndex:
	    return rfp->index;

	case vpiParent:
	    return parent;

	case vpiScope:
	    return vpi_handle(vpiScope, parent);

	case vpiModule:
	    return vpi_handle(vpiModule, parent);
      }

    return 0;
}

/*
static vpiHandle bit_iterate(int code, vpiHandle ref)
{
      struct __vpiBit*rfp = bit_from_handle(ref);
      assert(rfp);
      (void) code;

      return 0;
}

static vpiHandle bit_index(int idx, vpiHandle ref)
{
      struct __vpiBit*rfp = bit_from_handle(ref);
      assert(rfp);
      (void) idx;

      return 0;
}
*/

static void bit_get_value(vpiHandle ref, s_vpi_value*vp)
{
      struct __vpiBit*rfp = bit_from_handle(ref);
      assert(rfp);

      struct __vpiSignal*parent = rfp->get_parent();
      assert(parent);

      parent->get_bit_value(rfp, vp);
}

static vpiHandle bit_put_value(vpiHandle ref, s_vpi_value*vp, int flags)
{
      struct __vpiBit*rfp = bit_from_handle(ref);
      assert(rfp);

      struct __vpiSignal*parent = rfp->get_parent();
      assert(parent);

      return parent->put_bit_value(rfp, vp, flags);
}


int __vpiBit::get_index(void) const
{
      s_vpi_value vp;
      vp.format = vpiIntVal;
      vpi_get_value(index, &vp);
      return vp.value.integer;
}

int __vpiBit::as_bit_t::get_type_code(void) const
{ return bit_get_type(bit_from_handle(this)); }

int __vpiBit::as_bit_t::vpi_get(int code)
{ return bit_get(code, this); }

char* __vpiBit::as_bit_t::vpi_get_str(int code)
{ return bit_get_str(code, this); }

void __vpiBit::as_bit_t::vpi_get_value(p_vpi_value val)
{ bit_get_value(this, val); }

vpiHandle __vpiBit::as_bit_t::vpi_put_value(p_vpi_value val, int flags)
{ return bit_put_value(this, val, flags); }

vpiHandle __vpiBit::as_bit_t::vpi_handle(int code)
{ return bit_get_handle(code, this); }

// FIXME: How are delayed put values handled?
//vpiHandle __vpiBit::as_bit_t::vpi_iterate(int code) // FIXME: Is this needed?
//{ return bit_iterate(code, this); }

//vpiHandle __vpiBit::as_bit_t::vpi_index(int idx) // FIXME: Is this needed?
//{ return bit_index(idx, this); }
