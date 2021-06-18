/*
 * Copyright (c) 2008-2021 Stephen Williams (steve@icarus.com)
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

# include  "version_base.h"
# include  "vpi_priv.h"
# include  "schedule.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <vector>
# include  <cstdio>
# include  <cstdarg>
# include  <cstring>
# include  <cassert>
# include  <cstdlib>
# include  <cmath>
# include  <iostream>

using namespace std;
vpi_mode_t vpi_mode_flag = VPI_MODE_NONE;
FILE*vpi_trace = 0;

static s_vpi_vlog_info  vpi_vlog_info;
static s_vpi_error_info vpip_last_error = { 0, 0, 0, 0, 0, 0, 0 };

__vpiHandle::~__vpiHandle()
{ }

int __vpiHandle::vpi_get(int)
{ return vpiUndefined; }

char* __vpiHandle::vpi_get_str(int)
{ return 0; }

void __vpiHandle::vpi_get_value(p_vpi_value)
{ }

vpiHandle __vpiHandle::vpi_put_value(p_vpi_value, int)
{ return 0; }

vpiHandle __vpiHandle::vpi_handle(int)
{ return 0; }

vpiHandle __vpiHandle::vpi_iterate(int)
{ return 0; }

vpiHandle __vpiHandle::vpi_index(int)
{ return 0; }

void __vpiHandle::vpi_get_delays(p_vpi_delay)
{ }

void __vpiHandle::vpi_put_delays(p_vpi_delay)
{ }

__vpiBaseVar::__vpiBaseVar(__vpiScope*scope, const char*name, vvp_net_t*net)
: scope_(scope), name_(name), net_(net)
{
}

#ifdef CHECK_WITH_VALGRIND
__vpiBaseVar::~__vpiBaseVar()
{
      vvp_net_delete(net_);
}
#endif

/*
 * The default behavior for the vpi_free_object to an object is to
 * suppress the actual operation. This is because handles are
 * generally allocated semi-permanently within vvp context. Dynamic
 * objects will override the free_object_fun method to return an
 * appropriately effective function.
 */
static int suppress_free(vpiHandle)
{ return 1; }
__vpiHandle::free_object_fun_t __vpiHandle::free_object_fun(void)
{ return &suppress_free; }

/*
 * The vpip_string function creates a constant string from the pass
 * input. This constant string is permanently allocated from an
 * efficient string buffer store.
 */
struct vpip_string_chunk {
      struct vpip_string_chunk*next;
      char data[64*1024 - sizeof (struct vpip_string_chunk*)];
};

unsigned vpip_size(__vpiSignal *sig)
{
      return abs(sig->msb.get_value() - sig->lsb.get_value()) + 1;
}

__vpiScope* vpip_scope(__vpiSignal*sig)
{
      if (sig->is_netarray)
	    return static_cast<__vpiScope*>(vpi_handle(vpiScope,
	                                               sig->within.parent));
      else
	    return sig->within.scope;
}

__vpiScope* vpip_scope(__vpiRealVar*sig)
{
      if (sig->is_netarray)
	    return static_cast<__vpiScope*>(vpi_handle(vpiScope,
	                                               sig->within.parent));
      else
	    return sig->within.scope;
}

vpiHandle vpip_module(__vpiScope*scope)
{
      while(scope && scope->get_type_code() != vpiModule) {
	    scope = scope->scope;
      }
      assert(scope);
      return scope;
}

const char *vpip_string(const char*str)
{
      static vpip_string_chunk first_chunk = {0, {0}};
      static vpip_string_chunk*chunk_list = &first_chunk;
      static unsigned chunk_fill = 0;

      unsigned len = strlen(str);
      assert( (len+1) <= sizeof chunk_list->data );

      if ( (len+1) > (sizeof chunk_list->data - chunk_fill) ) {
	    vpip_string_chunk*tmp = new vpip_string_chunk;
	    tmp->next = chunk_list;
	    chunk_list = tmp;
	    chunk_fill = 0;
      }

      char*res = chunk_list->data + chunk_fill;
      chunk_fill += len + 1;

      strcpy(res, str);
      return res;
}

static unsigned hash_string(const char*text)
{
      unsigned h = 0;

      while (*text) {
	    h = (h << 4) ^ (h >> 28) ^ (unsigned)*text;
	    text += 1;
      }
      return h;
}

const char* vpip_name_string(const char*text)
{
      const unsigned HASH_SIZE = 4096;
      static const char*hash_table[HASH_SIZE] = {0};

      unsigned hash_value = hash_string(text) % HASH_SIZE;

	/* If we easily find the string in the hash table, then return
	   that and be done. */
      if (hash_table[hash_value]
	  && (strcmp(hash_table[hash_value], text) == 0)) {
	    return hash_table[hash_value];
      }

	/* The existing hash entry is not a match. Replace it with the
	   newly allocated value, and return the new pointer as the
	   result to the add. */
      const char*res = vpip_string(text);
      hash_table[hash_value] = res;

      return res;
}

PLI_INT32 vpi_chk_error(p_vpi_error_info info)
{
      if (vpip_last_error.state == 0)
	    return 0;

      info->state = vpip_last_error.state;
      info->level = vpip_last_error.level;
      info->message = vpip_last_error.message;
      info->product = vpi_vlog_info.product;
      info->code = (char *) "";
      info->file = 0;
      info->line = 0;

      return info->level;
}

PLI_INT32 vpi_compare_objects(vpiHandle obj1, vpiHandle obj2)
{
      assert(obj1);
      assert(obj2);

	// Does this actually work for all cases?
      if (obj1 != obj2) return 0;
      else return 1;
}

/*
 * Copy the internal information to the data structure. Do not free or
 * change the tfname/user_data since they are a pointer to the real
 * string/data values. We also support passing a task or function handle
 * instead of just a handle to a vpiUserSystf.
 */
void vpi_get_systf_info(vpiHandle ref, p_vpi_systf_data data)
{
      struct __vpiUserSystf* rfp = dynamic_cast<__vpiUserSystf*>(ref);
      if (rfp == 0) {
	    struct __vpiSysTaskCall*call = dynamic_cast<__vpiSysTaskCall*>(ref);
	    assert(call);
	    rfp = call->defn;
      }

	/* Assert that vpiUserDefn is true! */
      assert(rfp->is_user_defn);

      data->type = rfp->info.type;
      data->sysfunctype = rfp->info.sysfunctype;
      data->tfname = rfp->info.tfname;
      data->calltf = rfp->info.calltf;
      data->compiletf = rfp->info.compiletf;
      data->sizetf = rfp->info.sizetf;
      data->user_data = rfp->info.user_data;
}

/*
 * When a task is called, this value is set so that vpi_handle can
 * fathom the vpi_handle(vpiSysTfCall,0) function.
 */
struct __vpiSysTaskCall*vpip_cur_task = 0;

PLI_INT32 vpi_free_object(vpiHandle ref)
{
      int rtn;

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_free_object(%p)", ref);
	    fflush(vpi_trace);
      }

      assert(ref);
      __vpiHandle::free_object_fun_t fun = ref->free_object_fun();
      rtn = fun (ref);

      if (vpi_trace)
	    fprintf(vpi_trace, " --> %d\n", rtn);

      return rtn;
}

static int vpip_get_global(int property)
{
      switch (property) {

	  case vpiTimeUnit:
	  case vpiTimePrecision:
	    return vpip_get_time_precision();

	  default:
	    fprintf(stderr, "vpi error: bad global property: %d\n", property);
	    assert(0);
	    return vpiUndefined;
      }
}

static const char* vpi_property_str(PLI_INT32 code)
{
      static char buf[32];
      switch (code) {
	  case vpiConstType:
	    return "vpiConstType";
	  case vpiName:
	    return "vpiName";
	  case vpiFullName:
	    return "vpiFullName";
	  case vpiTimeUnit:
	    return "vpiTimeUnit";
	  case vpiTimePrecision:
	    return "vpiTimePrecision";
          case vpiSize:
	    return "vpiSize";
	  default:
	    sprintf(buf, "%d", (int)code);
      }
      return buf;
}

static const char* vpi_type_values(PLI_INT32 code)
{
      static char buf[32];
      switch (code) {
	  case vpiArrayType:
	    return "vpiArrayType";
	  case vpiBitVar:
	    return "vpiBitVar";
	  case vpiByteVar:
	    return "vpiByteVar";
	  case vpiClassVar:
	    return "vpiClassVar";
	  case vpiConstant:
	    return "vpiConstant";
	  case vpiEnumTypespec:
	    return "vpiEnumTypespec";
	  case vpiFunction:
	    return "vpiFunction";
	  case vpiGenScope:
	    return "vpiGenScope";
	  case vpiIntVar:
	    return "vpiIntVar";
	  case vpiIntegerVar:
	    return "vpiIntegerVar";
	  case vpiIterator:
	    return "vpiIterator";
	  case vpiLongIntVar:
	    return "vpiLongIntVar";
	  case vpiMemory:
	    return "vpiMemory";
	  case vpiMemoryWord:
	    return "vpiMemoryWord";
	  case vpiModule:
	    return "vpiModule";
	  case vpiNamedBegin:
	    return "vpiNamedBegin";
	  case vpiNamedEvent:
	    return "vpiNamedEvent";
	  case vpiNamedFork:
	    return "vpiNamedFork";
	  case vpiPackage:
	    return "vpiPackage";
	  case vpiPathTerm:
	    return "vpiPathTerm";
	  case vpiPort:
	    return "vpiPort";
	  case vpiNet:
	    return "vpiNet";
	  case vpiNetArray:
	    return "vpiNetArray";
	  case vpiNetBit:
	    return "vpiNetBit";
	  case vpiParameter:
	    return "vpiParameter";
	  case vpiPartSelect:
	    return "vpiPartSelect";
	  case vpiRealVar:
	    return "vpiRealVar";
	  case vpiReg:
	    return "vpiReg";
	  case vpiRegBit:
	    return "vpiRegBit";
	  case vpiShortIntVar:
	    return "vpiShortIntVar";
	  case vpiStringVar:
	    return "vpiStringVar";
	  case vpiSysFuncCall:
	    return "vpiSysFuncCall";
	  case vpiSysTaskCall:
	    return "vpiSysTaskCall";
	  case vpiTask:
	    return "vpiTask";
	  case vpiTimeVar:
	    return "vpiTimeVar";
	  case vpiUserSystf:
	    return "vpiUserSystf";
	  default:
	    sprintf(buf, "%d", (int)code);
      }
      return buf;
}

PLI_INT32 vpi_get(int property, vpiHandle ref)
{
	/* We don't care what the ref is there is only one delay selection. */
      if (property == _vpiDelaySelection) return vpip_delay_selection;

      if (ref == 0)
	    return vpip_get_global(property);

      if (property == vpiType) {
	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_get(vpiType, %p) --> %s\n",
			  ref, vpi_type_values(ref->get_type_code()));
	    }

	    if (ref->get_type_code() == vpiMemory && is_net_array(ref))
		  return vpiNetArray;
	    else
		  return ref->get_type_code();
      }

      int res = ref->vpi_get(property);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_get(%s, %p) --> %d\n",
		    vpi_property_str(property), ref, res);
      }

      return res;
}

char* vpi_get_str(PLI_INT32 property, vpiHandle ref)
{
	/* We don't care what the ref is there is only one delay selection. */
      if (property == _vpiDelaySelection) {
	    switch (vpip_delay_selection) {
		case _vpiDelaySelMinimum:
		  return simple_set_rbuf_str("MINIMUM");
		case _vpiDelaySelTypical:
		  return simple_set_rbuf_str("TYPICAL");
		case _vpiDelaySelMaximum:
		  return simple_set_rbuf_str("MAXIMUM");
		default:
		  assert(0);
	    }
      }

      if (ref == 0) {
	    fprintf(stderr, "vpi error: vpi_get_str(%s, 0) called "
		    "with null vpiHandle.\n", vpi_property_str(property));
	    return 0;
      }

      if (property == vpiType) {
	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_get(vpiType, %p) --> %s\n",
			  ref, vpi_type_values(ref->get_type_code()));
	    }

            PLI_INT32 type;
	    if (ref->get_type_code() == vpiMemory && is_net_array(ref))
		  type = vpiNetArray;
	    else
		  type = ref->get_type_code();
	    return (char *)vpi_type_values(type);
      }

      char*res = ref->vpi_get_str(property);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_get_str(%s, %p) --> %s\n",
		    vpi_property_str(property), ref, res? res : "<NULL>");
      }

      return res;
}

int vpip_time_units_from_handle(vpiHandle obj)
{
      struct __vpiSysTaskCall*task;
      __vpiScope*scope;
      struct __vpiSignal*signal;
      __vpiNamedEvent*event;

      if (obj == 0)
	    return vpip_get_time_precision();

      switch (obj->get_type_code()) {
	  case vpiSysTaskCall:
	    task = dynamic_cast<__vpiSysTaskCall*>(obj);
	    return task->scope->time_units;

	  case vpiModule:
	    scope = dynamic_cast<__vpiScope*>(obj);
	    return scope->time_units;

	  case vpiNet:
	  case vpiReg:
	    signal = dynamic_cast<__vpiSignal*>(obj);
	    scope = vpip_scope(signal);
	    return scope->time_units;

	  case vpiNamedEvent:
	    event = dynamic_cast<__vpiNamedEvent*>(obj);
	    scope = event->get_scope();
	    return scope->time_units;

	  default:
	    fprintf(stderr, "ERROR: vpip_time_units_from_handle called with "
		    "object handle type=%d\n", obj->get_type_code());
	    assert(0);
	    return 0;
      }
}

int vpip_time_precision_from_handle(vpiHandle obj)
{
      struct __vpiSysTaskCall*task;
      __vpiScope*scope;
      struct __vpiSignal*signal;

      if (obj == 0)
	    return vpip_get_time_precision();

      switch (obj->get_type_code()) {
	  case vpiSysTaskCall:
	    task = dynamic_cast<__vpiSysTaskCall*>(obj);
	    return task->scope->time_precision;

	  case vpiModule:
	    scope = dynamic_cast<__vpiScope*>(obj);
	    return scope->time_precision;

	  case vpiNet:
	  case vpiReg:
	    signal = dynamic_cast<__vpiSignal*>(obj);
	    scope = vpip_scope(signal);
	    return scope->time_precision;

	  default:
	    fprintf(stderr, "ERROR: vpip_time_precision_from_handle called "
		    "with object handle type=%d\n", obj->get_type_code());
	    assert(0);
	    return 0;
      }
}

void vpi_get_time(vpiHandle obj, s_vpi_time*vp)
{
      int scale;
      vvp_time64_t time;

      assert(vp);

      time = schedule_simtime();

      switch (vp->type) {
          case vpiSimTime:
	    vp->high = (time >> 32) & 0xffffffff;
	    vp->low = time & 0xffffffff;
	    break;

          case vpiScaledRealTime:
	    scale = vpip_get_time_precision() -
	            vpip_time_units_from_handle(obj);
	    if (scale >= 0) vp->real = (double)time * pow(10.0, scale);
	    else vp->real = (double)time / pow(10.0, -scale);
	    break;

          default:
            fprintf(stderr, "vpi_get_time: unknown type: %d\n", (int)vp->type);
            assert(0);
	    break;
      }
}

PLI_INT32 vpi_get_vlog_info(p_vpi_vlog_info vlog_info_p)
{
    if (vlog_info_p != 0) {
	  *vlog_info_p = vpi_vlog_info;
	  return 1;
    } else {
	  return 0;
    }
}

void vpi_set_vlog_info(int argc, char** argv)
{
    static char icarus_product[] = "Icarus Verilog";
    static char icarus_version[] = VERSION;
    vpi_vlog_info.product = icarus_product;
    vpi_vlog_info.version = icarus_version;
    vpi_vlog_info.argc    = argc;
    vpi_vlog_info.argv    = argv;

    static char trace_buf[1024];
    if (const char*path = getenv("VPI_TRACE"))  {
	  if (!strcmp(path,"-"))
		vpi_trace = stdout;
	  else {
		vpi_trace = fopen(path, "w");
		if (!vpi_trace) {
		      perror(path);
		      exit(1);
		}
		setvbuf(vpi_trace, trace_buf, _IOLBF, sizeof(trace_buf));
	  }
    }
}

static void vec4_get_value_string(const vvp_vector4_t&word_val, unsigned width,
				  s_vpi_value*vp)
{
      unsigned nchar = width / 8;
      unsigned tail = width % 8;

      char*rbuf = (char *) need_result_buf(nchar + 1, RBUF_VAL);
      char*cp = rbuf;

      if (tail > 0) {
	    char char_val = 0;
	    for (unsigned idx = width-tail; idx < width ;  idx += 1) {
		  vvp_bit4_t val = word_val.value(idx);
		  if (val == BIT4_1)
			char_val |= 1 << idx;
	    }

	    if (char_val != 0)
		  *cp++ = char_val;
      }

      for (unsigned idx = 0 ;  idx < nchar ;  idx += 1) {
	    unsigned bit = (nchar - idx - 1) * 8;
	    char char_val = 0;
	    for (unsigned bdx = 0 ;  bdx < 8 ;  bdx += 1) {
		  vvp_bit4_t val = word_val.value(bit+bdx);
		  if (val == BIT4_1)
			char_val |= 1 << bdx;
	    }
	    if (char_val != 0)
		  *cp++ = char_val;
      }

      *cp = 0;
      vp->value.str = rbuf;
}

/*
 * This is a generic function to convert a vvp_vector4_t value into a
 * vpi_value structure. The format is selected by the format of the
 * value pointer. The width is the real width of the word, in case the
 * word_val width is not accurate.
 */
void vpip_vec4_get_value(const vvp_vector4_t&word_val, unsigned width,
			 bool signed_flag, s_vpi_value*vp)
{
      char *rbuf = 0;

      switch (vp->format) {
	  default:
	    fprintf(stderr, "sorry: Format %d not implemented for "
	                    "getting vector values.\n", (int)vp->format);
	    assert(0);

	  case vpiSuppressVal:
	    break;

	  case vpiBinStrVal:
	    rbuf = (char *) need_result_buf(width+1, RBUF_VAL);
	    for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
		  vvp_bit4_t bit = word_val.value(idx);
		  rbuf[width-idx-1] = vvp_bit4_to_ascii(bit);
	    }
	    rbuf[width] = 0;
	    vp->value.str = rbuf;
	    break;

	  case vpiOctStrVal: {
		unsigned hwid = ((width+2) / 3) + 1;
		rbuf = (char *) need_result_buf(hwid, RBUF_VAL);
		vpip_vec4_to_oct_str(word_val, rbuf, hwid);
		vp->value.str = rbuf;
		break;
	  }

	  case vpiDecStrVal: {
// HERE need a better estimate.
		rbuf = (char *) need_result_buf(width+1, RBUF_VAL);
		vpip_vec4_to_dec_str(word_val, rbuf, width+1, signed_flag);
		vp->value.str = rbuf;
		break;
	  }

	  case vpiHexStrVal: {
		unsigned  hwid = ((width + 3) / 4) + 1;
		rbuf = (char *) need_result_buf(hwid, RBUF_VAL);
		vpip_vec4_to_hex_str(word_val, rbuf, hwid);
		vp->value.str = rbuf;
		break;
	  }

          case vpiScalarVal: {
	        // scalars should be of size 1
	        assert(width == 1);
	        switch(word_val.value(0)) {
		    case BIT4_0:
		      vp->value.scalar = vpi0;
		      break;
	            case BIT4_1:
                      vp->value.scalar = vpi1;
                      break;
	            case BIT4_X:
                      vp->value.scalar = vpiX;
                      break;
	            case BIT4_Z:
                      vp->value.scalar = vpiZ;
                      break;
		}
                break;
          }

	  case vpiIntVal: {
		long val = 0;
		vvp_bit4_t pad = BIT4_0;
		if (signed_flag && word_val.size() > 0)
		      pad = word_val.value(word_val.size()-1);

		for (unsigned idx = 0 ; idx < 8*sizeof(val) ;  idx += 1) {
		      vvp_bit4_t val4 = pad;
		      if (idx < word_val.size())
			    val4 = word_val.value(idx);
		      if (val4 == BIT4_1)
			    val |= 1L << idx;
		}

		vp->value.integer = val;
		break;
	  }

	  case vpiObjTypeVal:
	    // Use the following case to actually set the value!
	    vp->format = vpiVectorVal;
	    // fallthrough
	  case vpiVectorVal: {
		unsigned hwid = (width + 31)/32;

		s_vpi_vecval *op = (p_vpi_vecval)
			need_result_buf(hwid * sizeof(s_vpi_vecval), RBUF_VAL);
		vp->value.vector = op;

		op->aval = op->bval = 0;
		for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
		      switch (word_val.value(idx)) {
			  case BIT4_0:
			    op->aval &= ~(1 << idx % 32);
			    op->bval &= ~(1 << idx % 32);
			    break;
			  case BIT4_1:
			    op->aval |=  (1 << idx % 32);
			    op->bval &= ~(1 << idx % 32);
			    break;
			  case BIT4_X:
			    op->aval |= (1 << idx % 32);
			    op->bval |= (1 << idx % 32);
			    break;
			  case BIT4_Z:
			    op->aval &= ~(1 << idx % 32);
			    op->bval |=  (1 << idx % 32);
			    break;
		      }
		      if (!((idx+1) % 32) && (idx+1 < width)) {
			    op++;
			    op->aval = op->bval = 0;
		      }
		}
		break;
	  }

	  case vpiStringVal:
		vec4_get_value_string(word_val, width, vp);
		break;

	  case vpiRealVal:
		vector4_to_value(word_val, vp->value.real, signed_flag);
		break;
      }
}

void vpip_vec2_get_value(const vvp_vector2_t&word_val, unsigned width,
			 bool signed_flag, s_vpi_value*vp)
{
      switch (vp->format) {
	  default:
	    fprintf(stderr, "sorry: Format %d not implemented for "
	                    "getting vector2 values.\n", (int)vp->format);
	    assert(0);

	  case vpiSuppressVal:
	    break;

	  case vpiObjTypeVal:
	    vp->format = vpiIntVal;
	    // fallthrough
	  case vpiIntVal:
	    vector2_to_value(word_val, vp->value.integer, signed_flag);
	    break;

	  case vpiVectorVal: {
		unsigned hwid = (width + 31)/32;

		s_vpi_vecval *op = (p_vpi_vecval)
			need_result_buf(hwid * sizeof(s_vpi_vecval), RBUF_VAL);
		vp->value.vector = op;

		op->aval = op->bval = 0;
		for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
		      if (word_val.value(idx)) {
			    op->aval |=  (1 << idx % 32);
			    op->bval &= ~(1 << idx % 32);
		      } else {
			    op->aval &= ~(1 << idx % 32);
			    op->bval &= ~(1 << idx % 32);
		      }
		      if (!((idx+1) % 32) && (idx+1 < width)) {
			    op++;
			    op->aval = op->bval = 0;
		      }
		}
		break;
	  }
      }

}

/*
 * Convert a real value to the appropriate integer.
 */
static PLI_INT32 get_real_as_int(double real)
{
      double rtn;

	/* We would normally want to return 'bx for a NaN or
	 * +/- infinity, but for an integer the standard says
	 * to convert 'bx to 0, so we just return 0. */
      if (real != real || (real && (real == 0.5*real))) {
	    return 0;
      }

	/* Round away from zero. */
      if (real >= 0.0) {
	    rtn = floor(real);
	    if (real >= (rtn + 0.5)) rtn += 1.0;
      } else {
	    rtn = ceil(real);
	    if (real <= (rtn - 0.5)) rtn -= 1.0;
      }

      return (PLI_INT32) rtn;
}

/*
 * This is a generic function to convert a double value into a
 * vpi_value structure. The format is selected by the format of the
 * value pointer.
 */
void vpip_real_get_value(double real, s_vpi_value*vp)
{
      char *rbuf = 0;

      switch (vp->format) {
	  default:
	    fprintf(stderr, "sorry: Format %d not implemented for "
	                    "getting real values.\n", (int)vp->format);
	    assert(0);

	  case vpiSuppressVal:
	    break;

	  case vpiObjTypeVal:
	    // Use the following case to actually set the value!
	    vp->format = vpiRealVal;
	    // fallthrough
	  case vpiRealVal:
	    vp->value.real = real;
	    break;

	  case vpiIntVal:
	    vp->value.integer = get_real_as_int(real);
	    break;

	  case vpiDecStrVal:
	    rbuf = (char *) need_result_buf(1025, RBUF_VAL);
	    vpip_vec4_to_dec_str(vvp_vector4_t(1024, real), rbuf, 1025, true);
	    vp->value.str = rbuf;
	    break;
      }
}

double real_from_vpi_value(s_vpi_value*vp)
{
      vvp_vector4_t vec4(1024);
      double result;
      bool is_signed = false;

      switch (vp->format) {
	  default:
	    fprintf(stderr, "sorry: Format %d not implemented for "
	                    "putting real values.\n", (int)vp->format);
	    assert(0);

	  case vpiRealVal:
	    result = vp->value.real;
	    break;

	  case vpiIntVal:
	    result = (double) vp->value.integer;
	    break;

	  case vpiBinStrVal:
	    vpip_bin_str_to_vec4(vec4, vp->value.str);
	    if (vp->value.str[0] == '-') is_signed = true;
	    vector4_to_value(vec4, result, is_signed);
	    break;

	  case vpiOctStrVal:
	    vpip_oct_str_to_vec4(vec4, vp->value.str);
	    if (vp->value.str[0] == '-') is_signed = true;
	    vector4_to_value(vec4, result, is_signed);
	    break;

	  case vpiDecStrVal:
	    vpip_dec_str_to_vec4(vec4, vp->value.str);
	    if (vp->value.str[0] == '-') is_signed = true;
	    vector4_to_value(vec4, result, is_signed);
	    break;

	  case vpiHexStrVal:
	    vpip_hex_str_to_vec4(vec4, vp->value.str);
	    if (vp->value.str[0] == '-') is_signed = true;
	    vector4_to_value(vec4, result, is_signed);
	    break;

      }

      return result;
}

void vpip_string_get_value(const string&val, s_vpi_value*vp)
{
      char *rbuf = 0;

      switch (vp->format) {
	  default:
	    fprintf(stderr, "sorry: Format %d not implemented for "
	                    "getting string values.\n", (int)vp->format);
	    assert(0);

	  case vpiSuppressVal:
	    break;

	  case vpiObjTypeVal:
	    // Use the following case to actually set the value!
	    vp->format = vpiStringVal;
	    // fallthrough
	  case vpiStringVal:
	    rbuf = (char *) need_result_buf(val.size() + 1, RBUF_VAL);
	    strcpy(rbuf, val.c_str());
	    vp->value.str = rbuf;
	    break;
      }
}


void vpi_get_value(vpiHandle expr, s_vpi_value*vp)
{
      assert(expr);
      assert(vp);

	// Never bother with suppressed values. All the derived
	// classes can ignore this type.
      if (vp->format == vpiSuppressVal)
	    return;

      expr->vpi_get_value(vp);

      if (vpi_trace) switch (vp->format) {
		case vpiStringVal:
		  fprintf(vpi_trace,"vpi_get_value(%p=<%d>) -> string=\"%s\"\n",
			  expr, expr->get_type_code(), vp->value.str);
		  break;

		case vpiBinStrVal:
		  fprintf(vpi_trace, "vpi_get_value(<%d>...) -> binstr=%s\n",
			  expr->get_type_code(), vp->value.str);
		  break;

		case vpiIntVal:
		  fprintf(vpi_trace, "vpi_get_value(<%d>...) -> int=%d\n",
			  expr->get_type_code(), (int)vp->value.integer);
		  break;

		case vpiSuppressVal:
		  fprintf(vpi_trace, "vpi_get_value(<%d>...) -> <suppress>\n",
			  expr->get_type_code());
		  break;

		default:
		  fprintf(vpi_trace, "vpi_get_value(<%d>...) -> <%d>=?\n",
			  expr->get_type_code(), (int)vp->format);
	    }
}

struct vpip_put_value_event : vvp_gen_event_s {
      vpiHandle handle;
      s_vpi_value value;
      int flags;
      virtual void run_run();
      ~vpip_put_value_event() { }
};

void vpip_put_value_event::run_run()
{
      handle->vpi_put_value(&value, flags);
      switch (value.format) {
	    /* Free the copied string. */
	  case vpiBinStrVal:
	  case vpiOctStrVal:
	  case vpiDecStrVal:
	  case vpiHexStrVal:
	  case vpiStringVal:
	    free(value.value.str);
	    break;
	    /* Free the copied time structure. */
	  case vpiTimeVal:
	    free(value.value.time);
	    break;
	    /* Free the copied vector structure. */
	  case vpiVectorVal:
	    free(value.value.vector);
	    break;
	    /* Free the copied strength structure. */
	  case vpiStrengthVal:
	    free(value.value.strength);
	    break;
	    /* Everything else is static in the structure. */
	  default:
	    break;
      }
}

/* Make a copy of a pointer to a time structure. */
static t_vpi_time *timedup(t_vpi_time *val)
{
      t_vpi_time *rtn;
      rtn = static_cast<t_vpi_time *> (malloc(sizeof(t_vpi_time)));
      *rtn = *val;
      return rtn;
}

/* Make a copy of a pointer to a vector value structure. */
static t_vpi_vecval *vectordup(t_vpi_vecval *val, PLI_INT32 size)
{
      unsigned num_bytes;
      t_vpi_vecval *rtn;
      assert(size > 0);
      num_bytes = ((size + 31)/32)*sizeof(t_vpi_vecval);
      rtn = static_cast<t_vpi_vecval *> (malloc(num_bytes));
      memcpy(rtn, val, num_bytes);
      return rtn;
}

/* Make a copy of a pointer to a strength structure. */
static t_vpi_strengthval *strengthdup(t_vpi_strengthval *val)
{
      t_vpi_strengthval *rtn;
      rtn = static_cast<t_vpi_strengthval *>
            (malloc(sizeof(t_vpi_strengthval)));
      *rtn = *val;
      return rtn;
}

vpiHandle vpi_put_value(vpiHandle obj, s_vpi_value*vp,
			s_vpi_time*when, PLI_INT32 flags)
{
      assert(obj);

      flags &= ~vpiReturnEvent;

      if (flags!=vpiNoDelay && flags!=vpiForceFlag && flags!=vpiReleaseFlag) {
	    vvp_time64_t dly;
	    int scale;

	    if (vpi_get(vpiAutomatic, obj)) {
		  fprintf(stderr, "VPI error: cannot put a value with "
				  "a delay on automatically allocated "
				  "variable '%s'.\n",
				  vpi_get_str(vpiName, obj));
		  return 0;
	    }

	    assert(when != 0);

	    switch (when->type) {
		case vpiScaledRealTime:
		  scale = vpip_time_units_from_handle(obj) -
		          vpip_get_time_precision();
		  if (scale >= 0) {
			dly = (vvp_time64_t)(when->real * pow(10.0, scale));
		  } else {
			dly = (vvp_time64_t)(when->real / pow(10.0, -scale));
		  }
		  break;
		case vpiSimTime:
		  dly = vpip_timestruct_to_time(when);
		  break;
		default:
		  dly = 0;
		  break;
	    }

	    if ((dly == 0) && schedule_at_rosync()) {
		  fprintf(stderr, "VPI error: attempted to put a value to "
				  "variable '%s' during a read-only synch "
				  "callback.\n", vpi_get_str(vpiName, obj));
		  return 0;
	    }

	    vpip_put_value_event*put = new vpip_put_value_event;
	    put->handle = obj;
	    if (dynamic_cast<__vpiNamedEvent*>(obj)) {
		  put->value.format = vpiIntVal;
		  put->value.value.integer = 0;
	    } else {
		  assert(vp);
		  put->value = *vp;
	    }
	      /* Since this is a scheduled put event we must copy any pointer
	       * data to keep it available until the event is actually run. */
	    switch (put->value.format) {
		  /* Copy the string items. */
		case vpiBinStrVal:
		case vpiOctStrVal:
		case vpiDecStrVal:
		case vpiHexStrVal:
		case vpiStringVal:
		  put->value.value.str = strdup(put->value.value.str);
		  break;
		  /* Copy a time pointer item. */
		case vpiTimeVal:
		  put->value.value.time = timedup(put->value.value.time);
		  break;
		  /* Copy a vector pointer item. */
		case vpiVectorVal:
		  put->value.value.vector = vectordup(put->value.value.vector,
		                                      vpi_get(vpiSize, obj));
		  break;
		  /* Copy a strength pointer item. */
		case vpiStrengthVal:
		  put->value.value.strength =
		      strengthdup(put->value.value.strength);
		  break;
		  /* Everything thing else is already in the structure. */
		default:
		  break;
	    }
	    put->flags = flags;
	    schedule_generic(put, dly, false, true, true);
	    return 0;
      }

      if (schedule_at_rosync()) {
            fprintf(stderr, "VPI error: attempted to put a value to "
			    "variable '%s' during a read-only synch "
			    "callback.\n", vpi_get_str(vpiName, obj));
            return 0;
      }

      obj->vpi_put_value(vp, flags);

      return 0;
}

vpiHandle vpi_handle(PLI_INT32 type, vpiHandle ref)
{
      vpiHandle res = 0;

      if (ref == 0) {
	      // A few types can apply to a nil handle. These are ways
	      // that the current function can get started finding things.
	    switch (type) {

		case vpiScope:
		    // The IEEE1364-2005 doesn't seem to allow this,
		    // but some users seem to think it's handy, so
		    // return the scope that contains this SysTfCall.
		  assert(vpip_cur_task);
		  res = vpip_cur_task->vpi_handle(vpiScope);
		  break;

		case vpiSysTfCall:
		    // This is how VPI users get a first handle into
		    // the system. This is the handle of the system
		    // task/function call currently being executed.
		  if (vpi_trace) {
			fprintf(vpi_trace, "vpi_handle(vpiSysTfCall, 0) "
				"-> %p (%s)\n", vpip_cur_task,
				vpip_cur_task->defn->info.tfname);
		  }
		  return vpip_cur_task;

		default:
		  fprintf(stderr, "VPI error: vpi_handle(type=%d, ref=0).\n",
			  (int)type);
		  res = 0;
		  break;
	    }

      } else {

	    if (type == vpiSysTfCall) {
		  fprintf(stderr, "VPI error: vpi_handle(vpiSysTfCall, "
			  "ref!=0).\n");
		  return 0;
	    }

	    res = ref->vpi_handle(type);
      }


      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_handle(vpiScope, ref=%p) "
		    "-> %p\n", vpip_cur_task, ref);
      }

      return res;
}

static vpiHandle vpip_make_udp_iterator()
{
// HERE: Add support for iterating over UDP definitions.
//       See 26.6.16 (page 400 in 1364-2005).
      return 0;
}

/*
 * This function asks the object to return an iterator for
 * the specified reference. It is up to the iterate_ method to
 * allocate a properly formed iterator.
 */
static vpiHandle vpi_iterate_global(int type)
{
      switch (type) {
	  case vpiInstance:
	    // fallthrough
	  case vpiModule:
	    // fallthrough
	  case vpiProgram:
	    // fallthrough
	  case vpiInterface:
	    // fallthrough
	  case vpiPackage:
	    return vpip_make_root_iterator(type);

	  case vpiUdpDefn:
	    return vpip_make_udp_iterator();

	  case vpiUserSystf:
	    return vpip_make_systf_iterator();
      }

      return 0;
}

vpiHandle vpi_iterate(PLI_INT32 type, vpiHandle ref)
{
      vpiHandle rtn = 0;

      assert(vpi_mode_flag != VPI_MODE_NONE);
      if (vpi_mode_flag == VPI_MODE_REGISTER) {
	    fprintf(stderr, "vpi error: vpi_iterate called during "
		    "vpi_register_systf. You can't do that!\n");
	    return 0;
      }

      if (ref == 0)
	    rtn = vpi_iterate_global(type);
      else
	    rtn = ref->vpi_iterate(type);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_iterate(%d, %p) ->%s\n",
	    (int)type, ref, rtn ? "" : " (null)");
      }

      return rtn;
}

vpiHandle vpi_handle_by_index(vpiHandle ref, PLI_INT32 idx)
{
      assert(ref);
      return ref->vpi_index(idx);
}

static vpiHandle find_name(const char *name, vpiHandle handle)
{
      vpiHandle rtn = 0;
      __vpiScope*ref = dynamic_cast<__vpiScope*>(handle);

      /* check module names */
      if (!strcmp(name, vpi_get_str(vpiName, handle)))
	    rtn = handle;

      /* brute force search for the name in all objects in this scope */
      for (unsigned i = 0 ;  i < ref->intern.size() ;  i += 1) {
	      /* The standard says that since a port does not have a full
	       * name it cannot be found by name. Because of this we need
	       * to skip ports here so the correct handle can be located. */
	    if (vpi_get(vpiType, ref->intern[i]) == vpiPort) continue;
	    char *nm = vpi_get_str(vpiName, ref->intern[i]);
	    if (nm && !strcmp(name, nm)) {
		  rtn = ref->intern[i];
		  break;
	    } else if (vpi_get(vpiType, ref->intern[i]) == vpiMemory ||
	               vpi_get(vpiType, ref->intern[i]) == vpiNetArray) {
		  /* We need to iterate on the words */
		  vpiHandle word_i, word_h;
		  word_i = vpi_iterate(vpiMemoryWord, ref->intern[i]);
		  while (word_i && (word_h = vpi_scan(word_i))) {
			nm = vpi_get_str(vpiName, word_h);
			if (nm && !strcmp(name, nm)) {
			      rtn = word_h;
			      vpi_free_object(word_i);
			      break;
			}
		  }
	    }
	    /* found it yet? */
	    if (rtn) break;
      }

      return rtn;
}

// Find the end of the escaped identifier or simple identifier
static char * find_rest(char *name)
{
      char *rest;
      if (*name == '\\') {
	    rest = strchr(name, ' ');
	    if (rest) {
		  *rest++ = 0; // The space is not part of the string
		    // There should be a '.' after the escaped ID if there is
		    // anything more. If it is missing add it to avoid a crash
		  if ((*rest != '.') && (*rest != 0)) {
			*--rest = '.';
			fprintf(stderr, "ERROR: Malformed scope string: \"%s\"", name);
		  }
		  if (*rest == 0) {
			rest = 0;
		  }
	    }
      } else {
	    rest = strchr(name, '.');
      }
      return rest;
}

static vpiHandle find_scope(const char *name, vpiHandle handle, int depth)
{

      vpiHandle iter = handle==0
	    ? vpi_iterate(vpiModule, NULL)
	    : vpi_iterate(vpiInternalScope, handle);

      vector<char> name_buf (strlen(name)+1);
      strcpy(&name_buf[0], name);
      char*nm_first = &name_buf[0];
      char*nm_rest = find_rest(nm_first);
      if (*nm_first == '\\') ++nm_first;
      if (nm_rest) {
	    *nm_rest++ = 0;
      }

      vpiHandle rtn = 0;
      vpiHandle hand;
      while (iter && (hand = vpi_scan(iter))) {
	    char *nm = vpi_get_str(vpiName, hand);

	    if (strcmp(nm_first,nm)==0) {
		  if (nm_rest)
			rtn=find_scope(nm_rest, hand, depth+1);
		  else
			rtn = hand;
	    }

	    /* found it yet ? */
	    if (rtn) {
		  vpi_free_object(iter);
		  break;
	    }
      }

      return rtn;
}

// Find the end of the first escaped identifier or simple identifier
static char * find_next(char *name)
{
      char *next;
      if (*name == '\\') {
	    next = strchr(name, ' ');
	    if (next && *++next == 0) next = 0;
      } else {
	    next = strchr(name, '.');
      }
      return next;
}

vpiHandle vpi_handle_by_name(const char *name, vpiHandle scope)
{
      vpiHandle hand;

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_handle_by_name(%s, %p) -->\n",
		    name, scope);
      }

	// Chop the name into path and base. For example, if the name
	// is "a.b.c", then nm_path becomes "a.b" and nm_base becomes
	// "c". If the name is "c" then nm_path is nil and nm_base is "c".
      vector<char> name_buf (strlen(name)+1);
      strcpy(&name_buf[0], name);
      char*nm_path = &name_buf[0];
      char*nm_base;

	// Check to see if we have an escaped identifier and if so search
	// the long way because a '.' could be in the escaped name.
      if (strchr(nm_path, '\\')) {
	    char *next;
	    nm_base = nm_path;
	    while ((next = find_next(nm_base))) {
		  nm_base = ++next;
	    }
	    if (nm_path == nm_base) {
		  nm_path = 0;
	    } else {
		  *(nm_base-1) = 0;
	    }

	    if (*nm_base == '\\') {
		    // Skip the \ at the beginning
		  ++nm_base;

		    // Drop the space at the end if it exists
		  if ((next = strchr(nm_base, ' '))) {
			*next = 0;
		  }
	    }

	// If there is no escaped identifier then just look for the last '.'
      } else {
	    nm_base = strrchr(nm_path, '.');
	    if (nm_base) {
		  *nm_base++ = 0;
	    } else {
		  nm_base = nm_path;
		  nm_path = 0;
	    }
      }

      /* If scope provided, look in corresponding module; otherwise
       * traverse the hierarchy specified in name to find the leaf module
       * and try finding it there.
       */
      if (scope) {
	    /* Some implementations support either a module or a scope. */
	    switch (vpi_get(vpiType, scope)) {
		case vpiScope:
	          hand = vpi_handle(vpiModule, scope);
	          break;
		case vpiModule:
	          hand = scope;
	          break;
		default:
		  if (vpi_trace) {
			fprintf(vpi_trace, "vpi_handle_by_name: "
				"Scope is not a vpiScope or vpiModule\n");
		  }
	          // Use vpi_chk_error() here when it is implemented.
	          return 0;
	    }

      } else if (nm_path) {
	      // The name has a path, and no other scope handle was
	      // passed in. That suggests we are looking for "a.b.c"
	      // in the root scope. So convert "a.b" to a scope and
	      // start there to look for "c".
	    hand = find_scope(nm_path, NULL, 0);
	    nm_path = 0;

      } else {
	      // Special case: scope==<nil>, meaning we are looking in
	      // the root, and there is no path to the name, i.e. the
	      // string is "c" instead of "top.c". Try to find "c" as
	      // a scope and return that.
	    hand = find_scope(nm_base, NULL, 0);
      }

      if (hand == 0) {
	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_handle_by_name: "
			  "Scope does not exist. Giving up.\n");
	    }

	    return 0;
      }

	// If there is a path part, then use it to find the
	// scope. For example, if the full name is a.b.c, then
	// the nm_path string is a.b and we search for that
	// scope. If we find it, then set hand to that scope.
      if (nm_path) {
	    vpiHandle tmp = find_scope(nm_path, hand, 0);
	    while (tmp == 0 && hand != 0) {
		  hand = vpi_handle(vpiScope, hand);
		  tmp = find_scope(nm_path, hand, 0);
	    }
	    hand = tmp;
      }

	// Now we have the correct scope, look for the item.
      vpiHandle out = find_name(nm_base, hand);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_handle_by_name: DONE\n");
      }

      return out;
}


/*
  We increment the two vpi methods to enable the
  read/write of SDF delay values from/into
  the modpath vpiHandle


  basically, they will redirect the generic vpi_interface

  vpi_get_delay ( .. )
  vpi_put_delay ( .. )


  to the

  modpath_get_delay ( .. ) ;
  modpath_put_delay ( .. ) ;

*/



void vpi_get_delays(vpiHandle expr, p_vpi_delay delays)
{
      assert(expr);
      assert(delays);

      expr->vpi_get_delays(delays);

      if (vpi_trace) {
	    fprintf(vpi_trace,
		    "vpi_get_delays(%p, %p) -->\n", expr, delays);
      }
}


void vpi_put_delays(vpiHandle expr, p_vpi_delay delays)
{
      assert(expr  );
      assert(delays );

      expr->vpi_put_delays(delays);

      if (vpi_trace) {
	    fprintf(vpi_trace,
		    "vpi_put_delays(%p, %p) -->\n", expr, delays);
      }
}








extern "C" PLI_INT32 vpi_vprintf(const char*fmt, va_list ap)
{
      return vpi_mcd_vprintf(1, fmt, ap);
}

extern "C" PLI_INT32 vpi_printf(const char *fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      int r = vpi_mcd_vprintf(1, fmt, ap);
      va_end(ap);
      return r;
}

extern "C" PLI_INT32 vpi_flush(void)
{
      return vpi_mcd_flush(1);
}


extern "C" void vpi_sim_vcontrol(int operation, va_list ap)
{
      long diag_msg;

      switch (operation) {
	  case vpiFinish:
	  case __ivl_legacy_vpiFinish:
            diag_msg = va_arg(ap, long);
	    schedule_finish(diag_msg);
	    break;

	  case vpiStop:
	  case __ivl_legacy_vpiStop:
            diag_msg = va_arg(ap, long);
	    schedule_stop(diag_msg);
	    break;

	  default:
	    fprintf(stderr, "Unsupported operation %d.\n", operation);
	    assert(0);
      }
}

extern "C" void vpi_sim_control(PLI_INT32 operation, ...)
{
      va_list ap;
      va_start(ap, operation);
      vpi_sim_vcontrol(operation, ap);
      va_end(ap);
}

extern "C" void vpi_control(PLI_INT32 operation, ...)
{
      va_list ap;
      va_start(ap, operation);
      vpi_sim_vcontrol(operation, ap);
      va_end(ap);
}

/*
 * This routine calculated the return value for $clog2.
 * It is easier to do it here vs trying to to use the VPI interface.
 */
extern "C" s_vpi_vecval vpip_calc_clog2(vpiHandle arg)
{
      s_vpi_vecval rtn;
      s_vpi_value val;
      vvp_vector4_t vec4;
      bool is_neg = false;  // At this point only a real can be negative.

	/* Get the value as a vvp_vector4_t. */
      val.format = vpiObjTypeVal;
      vpi_get_value(arg, &val);
      if (val.format == vpiRealVal) {
	    vpi_get_value(arg, &val);
	      /* All double values can be represented in 1024 bits. */
	    vec4 = vvp_vector4_t(1024, val.value.real);
	    if (val.value.real < 0) is_neg = true;
      } else {
	    val.format = vpiVectorVal;
	    vpi_get_value(arg, &val);
	    unsigned wid = vpi_get(vpiSize, arg);
	    vec4 = vvp_vector4_t(wid, BIT4_0);
	    for (unsigned idx=0; idx < wid; idx += 1) {
		PLI_INT32 aval = val.value.vector[idx/32].aval;
		PLI_INT32 bval = val.value.vector[idx/32].bval;
		aval >>= idx % 32;
		bval >>= idx % 32;
		int bitmask = (aval&1) | ((bval<<1)&2);
		vvp_bit4_t bit = scalar_to_bit4(bitmask);
		vec4.set_bit(idx, bit);
	    }
      }

      if (vec4.has_xz()) {
	    rtn.aval = rtn.bval = 0xFFFFFFFFU;  /* Set to 'bx. */
	    return rtn;
      }

      vvp_vector2_t vec2(vec4);

      if (is_neg) vec2.trim_neg(); /* This is a special trim! */
      else vec2.trim(); /* This makes less work shifting. */

	/* Calculate the clog2 result. */
      PLI_INT32 res = 0;
      if (!vec2.is_zero()) {
	    vec2 -= vvp_vector2_t(1, vec2.size());
	    while(!vec2.is_zero()) {
		  res += 1;
		  vec2 >>= 1;
	    }
      }

      rtn.aval = res;
      rtn.bval = 0;
      return rtn;
}

/*
 * This routine provides the information needed to implement $countdrivers.
 * It is done here for performance reasons - interrogating the drivers
 * individually via the VPI interface would be much slower.
 */
extern "C" void vpip_count_drivers(vpiHandle ref, unsigned idx,
                                   unsigned counts[4])
{
      struct __vpiSignal*rfp = dynamic_cast<__vpiSignal*>(ref);
      assert(rfp);
      rfp->node->count_drivers(idx, counts);
}

#if defined(__MINGW32__) || defined (__CYGWIN__)
vpip_routines_s vpi_routines = {
    .register_cb                = vpi_register_cb,
    .remove_cb                  = vpi_remove_cb,
    .register_systf             = vpi_register_systf,
    .get_systf_info             = vpi_get_systf_info,
    .handle_by_name             = vpi_handle_by_name,
    .handle_by_index            = vpi_handle_by_index,
    .handle                     = vpi_handle,
    .iterate                    = vpi_iterate,
    .scan                       = vpi_scan,
    .get                        = vpi_get,
    .get_str                    = vpi_get_str,
    .get_delays                 = vpi_get_delays,
    .put_delays                 = vpi_put_delays,
    .get_value                  = vpi_get_value,
    .put_value                  = vpi_put_value,
    .get_time                   = vpi_get_time,
    .get_userdata               = vpi_get_userdata,
    .put_userdata               = vpi_put_userdata,
    .mcd_open                   = vpi_mcd_open,
    .mcd_close                  = vpi_mcd_close,
    .mcd_flush                  = vpi_mcd_flush,
    .mcd_name                   = vpi_mcd_name,
    .mcd_vprintf                = vpi_mcd_vprintf,
    .flush                      = vpi_flush,
    .vprintf                    = vpi_vprintf,
    .chk_error                  = vpi_chk_error,
    .compare_objects            = vpi_compare_objects,
    .free_object                = vpi_free_object,
    .get_vlog_info              = vpi_get_vlog_info,
    .vcontrol                   = vpi_sim_vcontrol,
    .fopen                      = vpi_fopen,
    .get_file                   = vpi_get_file,
    .calc_clog2                 = vpip_calc_clog2,
    .count_drivers              = vpip_count_drivers,
    .format_strength            = vpip_format_strength,
    .make_systf_system_defined  = vpip_make_systf_system_defined,
    .mcd_rawwrite               = vpip_mcd_rawwrite,
    .set_return_value           = vpip_set_return_value,
};
#endif
