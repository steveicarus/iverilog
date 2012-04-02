/*
 * Copyright (c) 2008-2012 Stephen Williams (steve@icarus.com)
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

# include  "version_base.h"
# include  "vpi_priv.h"
# include  "schedule.h"
# include  <cstdio>
# include  <cstdarg>
# include  <cstring>
# include  <cassert>
# include  <cstdlib>
# include  <cmath>

vpi_mode_t vpi_mode_flag = VPI_MODE_NONE;
FILE*vpi_trace = 0;

static s_vpi_vlog_info  vpi_vlog_info;
static s_vpi_error_info vpip_last_error = { 0, 0, 0, 0, 0, 0, 0 };

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
      return abs(sig->msb - sig->lsb) + 1;
}

struct __vpiScope* vpip_scope(__vpiSignal*sig)
{
      if (sig->is_netarray)
	    return  (struct __vpiScope*) vpi_handle(vpiScope, sig->within.parent);
      else
	    return sig->within.scope;
}

struct __vpiScope* vpip_scope(__vpiRealVar*sig)
{
      if (sig->is_netarray)
	    return  (struct __vpiScope*) vpi_handle(vpiScope, sig->within.parent);
      else
	    return sig->within.scope;
}

vpiHandle vpip_module(struct __vpiScope*scope)
{
      while(scope && scope->base.vpi_type->type_code != vpiModule) {
	    scope = scope->scope;
      }
      assert(scope);
      return &scope->base;
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
	    h = (h << 4) ^ (h >> 28) ^ *text;
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
      if (ref->vpi_type->vpi_free_object_ == 0)
	    rtn = 1;
      else
	    rtn = ref->vpi_type->vpi_free_object_(ref);

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
	  case vpiConstant:
	    return "vpiConstant";
	  case vpiFunction:
	    return "vpiFunction";
	  case vpiIntegerVar:
	    return "vpiIntegerVar";
	  case vpiIterator:
	    return "vpiIterator";
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
	  case vpiNet:
	    return "vpiNet";
	  case vpiNetArray:
	    return "vpiNetArray";
	  case vpiParameter:
	    return "vpiParameter";
	  case vpiPartSelect:
	    return "vpiPartSelect";
	  case vpiRealVar:
	    return "vpiRealVar";
	  case vpiReg:
	    return "vpiReg";
	  case vpiSysFuncCall:
	    return "vpiSysFuncCall";
	  case vpiSysTaskCall:
	    return "vpiSysTaskCall";
	  case vpiTask:
	    return "vpiTask";
	  case vpiTimeVar:
	    return "vpiTimeVar";
	  default:
	    sprintf(buf, "%d", (int)code);
      }
      return buf;
}

PLI_INT32 vpi_get(int property, vpiHandle ref)
{
      if (ref == 0)
	    return vpip_get_global(property);

      if (property == vpiType) {
	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_get(vpiType, %p) --> %s\n",
			  ref, vpi_type_values(ref->vpi_type->type_code));
	    }

	    struct __vpiSignal*rfp = (struct __vpiSignal*)ref;
	    if (ref->vpi_type->type_code == vpiReg && rfp->isint_)
		  return vpiIntegerVar;
	    else if (ref->vpi_type->type_code == vpiMemory && is_net_array(ref))
		  return vpiNetArray;
	    else
		  return ref->vpi_type->type_code;
      }

      if (ref->vpi_type->vpi_get_ == 0) {
	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_get(%s, %p) --X\n",
			  vpi_property_str(property), ref);
	    }

	    return vpiUndefined;
      }

      int res = (ref->vpi_type->vpi_get_)(property, ref);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_get(%s, %p) --> %d\n",
		    vpi_property_str(property), ref, res);
      }

      return res;
}

char* vpi_get_str(PLI_INT32 property, vpiHandle ref)
{
      if (ref == 0) {
	    fprintf(stderr, "vpi error: vpi_get_str(%s, 0) called "
		    "with null vpiHandle.\n", vpi_property_str(property));
	    return 0;
      }

      if (property == vpiType) {
	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_get(vpiType, %p) --> %s\n",
			  ref, vpi_type_values(ref->vpi_type->type_code));
	    }

	    struct __vpiSignal*rfp = (struct __vpiSignal*)ref;
            PLI_INT32 type;
	    if (ref->vpi_type->type_code == vpiReg && rfp->isint_)
		  type = vpiIntegerVar;
	    else if (ref->vpi_type->type_code == vpiMemory && is_net_array(ref))
		  type = vpiNetArray;
	    else
		  type = ref->vpi_type->type_code;
	    return (char *)vpi_type_values(type);
      }

      if (ref->vpi_type->vpi_get_str_ == 0) {
	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_get_str(%s, %p) --X\n",
			  vpi_property_str(property), ref);
	    }
	    return 0;
      }

      char*res = (char*)(ref->vpi_type->vpi_get_str_)(property, ref);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_get_str(%s, %p) --> %s\n",
		    vpi_property_str(property), ref, res);
      }

      return res;
}

int vpip_time_units_from_handle(vpiHandle obj)
{
      struct __vpiSysTaskCall*task;
      struct __vpiScope*scope;
      struct __vpiSignal*signal;

      if (obj == 0)
	    return vpip_get_time_precision();

      switch (obj->vpi_type->type_code) {
	  case vpiSysTaskCall:
	    task = (struct __vpiSysTaskCall*)obj;
	    return task->scope->time_units;

	  case vpiModule:
	    scope = (struct __vpiScope*)obj;
	    return scope->time_units;

	  case vpiNet:
	  case vpiReg:
	    signal = vpip_signal_from_handle(obj);
	    scope = vpip_scope(signal);
	    return scope->time_units;

	  default:
	    fprintf(stderr, "ERROR: vpip_time_units_from_handle called with "
		    "object handle type=%u\n", obj->vpi_type->type_code);
	    assert(0);
	    return 0;
      }
}

int vpip_time_precision_from_handle(vpiHandle obj)
{
      struct __vpiSysTaskCall*task;
      struct __vpiScope*scope;
      struct __vpiSignal*signal;

      if (obj == 0)
	    return vpip_get_time_precision();

      switch (obj->vpi_type->type_code) {
	  case vpiSysTaskCall:
	    task = (struct __vpiSysTaskCall*)obj;
	    return task->scope->time_precision;

	  case vpiModule:
	    scope = (struct __vpiScope*)obj;
	    return scope->time_precision;

	  case vpiNet:
	  case vpiReg:
	    signal = vpip_signal_from_handle(obj);
	    scope = vpip_scope(signal);
	    return scope->time_precision;

	  default:
	    fprintf(stderr, "ERROR: vpip_time_precision_from_handle called "
		    "with object handle type=%u\n", obj->vpi_type->type_code);
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

    if (const char*path = getenv("VPI_TRACE")) {
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

      char*rbuf = need_result_buf(nchar + 1, RBUF_VAL);
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
	    rbuf = need_result_buf(width+1, RBUF_VAL);
	    for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
		  vvp_bit4_t bit = word_val.value(idx);
		  rbuf[width-idx-1] = vvp_bit4_to_ascii(bit);
	    }
	    rbuf[width] = 0;
	    vp->value.str = rbuf;
	    break;

	  case vpiOctStrVal: {
		unsigned hwid = (width+2) / 3;
		rbuf = need_result_buf(hwid+1, RBUF_VAL);
		vpip_vec4_to_oct_str(word_val, rbuf, hwid+1, signed_flag);
		vp->value.str = rbuf;
		break;
	  }

	  case vpiDecStrVal: {
		rbuf = need_result_buf(width+1, RBUF_VAL);
		vpip_vec4_to_dec_str(word_val, rbuf, width+1, signed_flag);
		vp->value.str = rbuf;
		break;
	  }

	  case vpiHexStrVal: {
		unsigned  hwid = (width + 3) / 4;

		rbuf = need_result_buf(hwid+1, RBUF_VAL);
		rbuf[hwid] = 0;

		vpip_vec4_to_hex_str(word_val, rbuf, hwid+1, signed_flag);
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

	  case vpiVectorVal: {
		unsigned hwid = (width - 1)/32 + 1;

		rbuf = need_result_buf(hwid * sizeof(s_vpi_vecval), RBUF_VAL);
		s_vpi_vecval *op = (p_vpi_vecval)rbuf;
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

	  case vpiRealVal:
	    vp->value.real = real;
	    break;

	  case vpiIntVal:
	    vp->value.integer = get_real_as_int(real);
	    break;

	  case vpiDecStrVal:
	    rbuf = need_result_buf(1025, RBUF_VAL);
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

void vpi_get_value(vpiHandle expr, s_vpi_value*vp)
{
      assert(expr);
      assert(vp);
      if (expr->vpi_type->vpi_get_value_) {
	    (expr->vpi_type->vpi_get_value_)(expr, vp);

	    if (vpi_trace) switch (vp->format) {
		case vpiStringVal:
		  fprintf(vpi_trace,"vpi_get_value(%p=<%d>) -> string=\"%s\"\n",
			  expr, expr->vpi_type->type_code, vp->value.str);
		  break;

		case vpiBinStrVal:
		  fprintf(vpi_trace, "vpi_get_value(<%d>...) -> binstr=%s\n",
			  expr->vpi_type->type_code, vp->value.str);
		  break;

		case vpiIntVal:
		  fprintf(vpi_trace, "vpi_get_value(<%d>...) -> int=%d\n",
			  expr->vpi_type->type_code, (int)vp->value.integer);
		  break;

		default:
		  fprintf(vpi_trace, "vpi_get_value(<%d>...) -> <%d>=?\n",
			  expr->vpi_type->type_code, (int)vp->format);
	    }
	    return;
      }

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_get_value(<%d>...) -> <suppress>\n",
		    expr->vpi_type->type_code);
      }

      vp->format = vpiSuppressVal;
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
      handle->vpi_type->vpi_put_value_ (handle, &value, flags);
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
      rtn = (t_vpi_time *) malloc(sizeof(t_vpi_time));
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
      rtn = (t_vpi_vecval *) malloc(num_bytes);
      memcpy(rtn, val, num_bytes);
      return rtn;
}

/* Make a copy of a pointer to a strength structure. */
static t_vpi_strengthval *strengthdup(t_vpi_strengthval *val)
{
      t_vpi_strengthval *rtn;
      rtn = (t_vpi_strengthval *) malloc(sizeof(t_vpi_strengthval));
      *rtn = *val;
      return rtn;
}

vpiHandle vpi_put_value(vpiHandle obj, s_vpi_value*vp,
			s_vpi_time*when, PLI_INT32 flags)
{
      assert(obj);

      if (obj->vpi_type->vpi_put_value_ == 0)
	    return 0;

      flags &= ~vpiReturnEvent;

      if (flags!=vpiNoDelay && flags!=vpiForceFlag && flags!=vpiReleaseFlag) {
	    vvp_time64_t dly;
	    int scale;

            if (vpi_get(vpiAutomatic, obj)) {
                  fprintf(stderr, "vpi error: cannot put a value with "
                                  "a delay on automatically allocated "
                                  "variable '%s'\n",
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

	    vpip_put_value_event*put = new vpip_put_value_event;
	    put->handle = obj;
	    put->value = *vp;
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

      (obj->vpi_type->vpi_put_value_)(obj, vp, flags);

      return 0;
}

vpiHandle vpi_handle(PLI_INT32 type, vpiHandle ref)
{
      if (type == vpiSysTfCall) {
	    assert(ref == 0);

	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_handle(vpiSysTfCall, 0) "
			  "-> %p (%s)\n", &vpip_cur_task->base,
			  vpip_cur_task->defn->info.tfname);
	    }

	    return &vpip_cur_task->base;
      }

      if (ref == 0) {
	    fprintf(stderr, "internal error: vpi_handle(type=%d, ref=0)\n",
		    (int)type);
      }
      assert(ref);

      if (ref->vpi_type->handle_ == 0) {

	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_handle(%d, %p) -X\n",
			  (int)type, ref);
	    }

	    return 0;
      }

      assert(ref->vpi_type->handle_);
      vpiHandle res = (ref->vpi_type->handle_)(type, ref);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_handle(%d, %p) -> %p\n",
		    (int)type, ref, res);
      }

      return res;
}

/*
 * This function asks the object to return an iterator for
 * the specified reference. It is up to the iterate_ method to
 * allocate a properly formed iterator.
 */
static vpiHandle vpi_iterate_global(int type)
{
      switch (type) {
	  case vpiModule:
	    return vpip_make_root_iterator();

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
      else if (ref->vpi_type->iterate_)
	    rtn = (ref->vpi_type->iterate_)(type, ref);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_iterate(%d, %p) ->%s\n",
	    (int)type, ref, rtn ? "" : " (null)");
      }

      return rtn;
}

vpiHandle vpi_handle_by_index(vpiHandle ref, PLI_INT32 idx)
{
      assert(ref);

      if (ref->vpi_type->index_ == 0)
	    return 0;

      assert(ref->vpi_type->index_);
      return (ref->vpi_type->index_)(ref, idx);
}

static vpiHandle find_name(const char *name, vpiHandle handle)
{
      vpiHandle rtn = 0;
      struct __vpiScope*ref = (struct __vpiScope*)handle;

      /* check module names */
      if (!strcmp(name, vpi_get_str(vpiName, handle)))
	    rtn = handle;

      /* brute force search for the name in all objects in this scope */
      for (unsigned i = 0 ;  i < ref->nintern ;  i += 1) {
	    char *nm = vpi_get_str(vpiName, ref->intern[i]);
	    if (!strcmp(name, nm)) {
		  rtn = ref->intern[i];
		  break;
	    } else if (vpi_get(vpiType, ref->intern[i]) == vpiMemory ||
	               vpi_get(vpiType, ref->intern[i]) == vpiNetArray) {
		  /* We need to iterate on the words */
		  vpiHandle word_i, word_h;
		  word_i = vpi_iterate(vpiMemoryWord, ref->intern[i]);
		  while (word_i && (word_h = vpi_scan(word_i))) {
			nm = vpi_get_str(vpiName, word_h);
			if (!strcmp(name, nm)) {
			      rtn = word_h;
			      break;
			}
		  }
	    }
	    /* found it yet? */
	    if (rtn) break;
      }

      return rtn;
}

static vpiHandle find_scope(const char *name, vpiHandle handle, int depth)
{
      vpiHandle iter, hand, rtn = 0;

      iter = !handle ? vpi_iterate(vpiModule, NULL) :
		       vpi_iterate(vpiInternalScope, handle);

      while (iter && (hand = vpi_scan(iter))) {
	    char *nm = vpi_get_str(vpiName, hand);
	    int len = strlen(nm);
	    const char *cp = name + len;	/* hier separator */

	    if (!handle && !strcmp(name, nm)) {
		  /* root module */
		  rtn = hand;
	    } else if (!strncmp(name, nm, len) && *(cp) == '.')
		  /* recurse deeper */
		  rtn = find_scope(cp+1, hand, depth + 1);

	    /* found it yet ? */
	    if (rtn) {
		  vpi_free_object(iter);
		  break;
	    }
      }

      /* matched up to here */
      if (!rtn) rtn = handle;

      return rtn;
}

vpiHandle vpi_handle_by_name(const char *name, vpiHandle scope)
{
      vpiHandle hand;
      const char *nm, *cp;
      int len;


      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_handle_by_name(%s, %p) -->\n",
		    name, scope);
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
	          // Use vpi_chk_error() here when it is implemented.
	          return 0;
	    }
      } else {
	    hand = find_scope(name, NULL, 0);
      }

      if (hand) {
	    /* remove hierarchical portion of name */
	    nm = vpi_get_str(vpiFullName, hand);
	    len = strlen(nm);
	    cp = name + len;
	    if (!strncmp(name, nm, len) && *cp == '.') name = cp + 1;

	    /* Ok, time to burn some cycles */
	    vpiHandle out = find_name(name, hand);
	    return out;
      }

      return 0;
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

      if (expr->vpi_type->vpi_get_delays_)
	{
	  (expr->vpi_type->vpi_get_delays_)(expr, delays);

	  if (vpi_trace)
	    {
	      fprintf(vpi_trace,
		      "vpi_get_delays(%p, %p) -->\n", expr, delays);
	    }
	}
}


void vpi_put_delays(vpiHandle expr, p_vpi_delay delays)
{
      assert(expr  );
      assert(delays );

      if (expr->vpi_type->vpi_put_delays_)
	{
	  (expr->vpi_type->vpi_put_delays_)(expr, delays);

	  if (vpi_trace)
	    {
	      fprintf(vpi_trace,
		      "vpi_put_delays(%p, %p) -->\n", expr, delays);
	    }
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
