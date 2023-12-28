/*
 * Copyright (c) 2003-2023 Michael Ruff (mruff  at chiaro.com)
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

# include  "sys_priv.h"
# include  <stdio.h>
# include  <string.h>
# include  <math.h>

static double bits2double(PLI_UINT32 bits[2])
{
      union conv {
	    double rval;
	    unsigned char bval[sizeof(double)];
      } conv;

#ifdef WORDS_BIGENDIAN
      conv.bval[7] = (bits[0] >>  0) & 0xff;
      conv.bval[6] = (bits[0] >>  8) & 0xff;
      conv.bval[5] = (bits[0] >> 16) & 0xff;
      conv.bval[4] = (bits[0] >> 24) & 0xff;
      conv.bval[3] = (bits[1] >>  0) & 0xff;
      conv.bval[2] = (bits[1] >>  8) & 0xff;
      conv.bval[1] = (bits[1] >> 16) & 0xff;
      conv.bval[0] = (bits[1] >> 24) & 0xff;
#else
      conv.bval[0] = (bits[0] >>  0) & 0xff;
      conv.bval[1] = (bits[0] >>  8) & 0xff;
      conv.bval[2] = (bits[0] >> 16) & 0xff;
      conv.bval[3] = (bits[0] >> 24) & 0xff;
      conv.bval[4] = (bits[1] >>  0) & 0xff;
      conv.bval[5] = (bits[1] >>  8) & 0xff;
      conv.bval[6] = (bits[1] >> 16) & 0xff;
      conv.bval[7] = (bits[1] >> 24) & 0xff;
#endif

      return conv.rval;
}

static void double2bits(double real, PLI_UINT32 bits[2])
{
      union conv {
	    double rval;
	    unsigned char bval[sizeof(double)];
      } conv;

      conv.rval = real;

#ifdef WORDS_BIGENDIAN
      bits[0] = (conv.bval[7] <<  0)
              | (conv.bval[6] <<  8)
              | (conv.bval[5] << 16)
              | (conv.bval[4] << 24);
      bits[1] = (conv.bval[3] <<  0)
              | (conv.bval[2] <<  8)
              | (conv.bval[1] << 16)
              | (conv.bval[0] << 24);
#else
      bits[0] = (conv.bval[0] <<  0)
              | (conv.bval[1] <<  8)
              | (conv.bval[2] << 16)
              | (conv.bval[3] << 24);
      bits[1] = (conv.bval[4] <<  0)
              | (conv.bval[5] <<  8)
              | (conv.bval[6] << 16)
              | (conv.bval[7] << 24);
#endif
}

static float bits2float(PLI_UINT32 bits)
{
      union conv {
	    float rval;
	    unsigned char bval[sizeof(float)];
      } conv;

#ifdef WORDS_BIGENDIAN
      conv.bval[3] = (bits >>  0) & 0xff;
      conv.bval[2] = (bits >>  8) & 0xff;
      conv.bval[1] = (bits >> 16) & 0xff;
      conv.bval[0] = (bits >> 24) & 0xff;
#else
      conv.bval[0] = (bits >>  0) & 0xff;
      conv.bval[1] = (bits >>  8) & 0xff;
      conv.bval[2] = (bits >> 16) & 0xff;
      conv.bval[3] = (bits >> 24) & 0xff;
#endif

      return conv.rval;
}

static void float2bits(float real, PLI_UINT32*bits)
{
      union conv {
	    float rval;
	    unsigned char bval[sizeof(float)];
      } conv;

      conv.rval = real;

#ifdef WORDS_BIGENDIAN
      *bits = (conv.bval[3] <<  0)
            | (conv.bval[2] <<  8)
            | (conv.bval[1] << 16)
            | (conv.bval[0] << 24);
#else
      *bits = (conv.bval[0] <<  0)
            | (conv.bval[1] <<  8)
            | (conv.bval[2] << 16)
            | (conv.bval[3] << 24);
#endif
}

static void error_message(vpiHandle callh, const char* msg)
{
    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
    vpi_printf(msg, vpi_get_str(vpiName, callh));
    vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
}

static PLI_INT32 sizetf_64 (ICARUS_VPI_CONST PLI_BYTE8*name)
{
    (void)name;  /* Parameter is not used. */
    return 64;
}

static PLI_INT32 sizetf_32 (ICARUS_VPI_CONST PLI_BYTE8*name)
{
    (void)name;  /* Parameter is not used. */
    return 32;
}

static PLI_INT32 sys_convert_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle arg;

    /* Check that there is an argument. */
    if (argv == 0) {
	error_message(callh, "%s requires one argument.\n");
	return 0;
    }

    /* In Icarus if we have an argv we have at least one argument. */
    arg  = vpi_scan(argv);

    /* Validate the argument. Only $bitstoreal and $bitstoshortreal for now. */
    if (!strcmp("$bitstoreal", name) && vpi_get(vpiSize, arg) != 64) {
	error_message(callh, "%s requires a 64-bit argument.\n");
	return 0;
    }
    if (!strcmp("$bitstoshortreal", name) && vpi_get(vpiSize, arg) != 32) {
	error_message(callh, "%s requires a 32-bit argument.\n");
	return 0;
    }

    /* Save the argument away to make the calltf faster. */
    vpi_put_userdata(callh, (void *) arg);

    /* These functions only take one argument. */
    check_for_extra_args(argv, callh, name, "one argument", 0);

    return 0;
}

static PLI_INT32 sys_bitstoreal_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle arg  = (vpiHandle) vpi_get_userdata(callh);
    s_vpi_value value;

    PLI_UINT32 bits[2];

    (void)name;  /* Parameter is not used. */

    /* get value */
    value.format = vpiVectorVal;
    vpi_get_value(arg, &value);

    /* convert */
    bits[0] = (value.value.vector[0]).aval;
    bits[1] = (value.value.vector[1]).aval;
    value.value.real = bits2double(bits);
    value.format = vpiRealVal;

    /* return converted value */
    vpi_put_value(callh, &value, 0, vpiNoDelay);

    return 0;
}

static PLI_INT32 sys_bitstoshortreal_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle arg  = (vpiHandle) vpi_get_userdata(callh);
    s_vpi_value value;

    PLI_UINT32 bits;

    (void)name;  /* Parameter is not used. */

    /* get value */
    value.format = vpiVectorVal;
    vpi_get_value(arg, &value);

    /* convert */
    bits = (value.value.vector[0]).aval;
    value.value.real = bits2float(bits);
    value.format = vpiRealVal;

    /* return converted value */
    vpi_put_value(callh, &value, 0, vpiNoDelay);

    return 0;
}

static PLI_INT32 sys_itor_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle arg  = (vpiHandle) vpi_get_userdata(callh);
    s_vpi_value value;

    (void)name;  /* Parameter is not used. */

    /* get value */
    value.format = vpiIntVal;
    vpi_get_value(arg, &value);

    /* convert */
    value.value.real = value.value.integer;
    value.format = vpiRealVal;

    /* return converted value */
    vpi_put_value(callh, &value, 0, vpiNoDelay);

    return 0;
}

static PLI_INT32 sys_realtobits_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle arg  = (vpiHandle) vpi_get_userdata(callh);
    s_vpi_value value;
    static struct t_vpi_vecval res[2];

    PLI_UINT32 bits[2];

    (void)name;  /* Parameter is not used. */

    /* get value */
    value.format = vpiRealVal;
    vpi_get_value(arg, &value);

    /* convert */
    double2bits(value.value.real, bits);

    res[0].aval = bits[0];
    res[0].bval = 0;
    res[1].aval = bits[1];
    res[1].bval = 0;

    value.format = vpiVectorVal;
    value.value.vector = res;

    /* return converted value */
    vpi_put_value(callh, &value, 0, vpiNoDelay);

    return 0;
}

static PLI_INT32 sys_shortrealtobits_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle arg  = (vpiHandle) vpi_get_userdata(callh);
    s_vpi_value value;
    static struct t_vpi_vecval res[1];

    PLI_UINT32 bits;

    (void)name;  /* Parameter is not used. */

    /* get value */
    value.format = vpiRealVal;
    vpi_get_value(arg, &value);

    /* convert */
    float2bits(value.value.real, &bits);

    res[0].aval = bits;
    res[0].bval = 0;

    value.format = vpiVectorVal;
    value.value.vector = res;

    /* return converted value */
    vpi_put_value(callh, &value, 0, vpiNoDelay);

    return 0;
}

static PLI_INT32 sys_rtoi_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle arg  = (vpiHandle) vpi_get_userdata(callh);
    s_vpi_value value;
    static struct t_vpi_vecval res;
    double val;

    (void)name;  /* Parameter is not used. */

    /* get value */
    value.format = vpiRealVal;
    vpi_get_value(arg, &value);

    /* If the value is NaN or +/- infinity then return 'bx */
    val = value.value.real;
    if (val != val || (val && (val == 0.5*val))) {
	res.aval = ~(PLI_INT32)0;
	res.bval = ~(PLI_INT32)0;
    } else {
	/* This is not 100% correct since large real values may break this
	 * code. See the verinum code for a more rigorous implementation. */
	if (val >= 0.0) res.aval = (PLI_UINT64) val;
	else res.aval = - (PLI_UINT64) -val;
	res.bval = 0;
    }

    value.format = vpiVectorVal;
    value.value.vector = &res;

    /* return converted value */
    vpi_put_value(callh, &value, 0, vpiNoDelay);

    return 0;
}

void sys_convert_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiRealFunc;
      tf_data.user_data   = "$bitstoreal";
      tf_data.tfname      = tf_data.user_data;
      tf_data.sizetf      = 0;
      tf_data.compiletf   = sys_convert_compiletf;
      tf_data.calltf      = sys_bitstoreal_calltf;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiRealFunc;
      tf_data.user_data   = "$bitstoshortreal";
      tf_data.tfname      = tf_data.user_data;
      tf_data.sizetf      = 0;
      tf_data.compiletf   = sys_convert_compiletf;
      tf_data.calltf      = sys_bitstoshortreal_calltf;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiRealFunc;
      tf_data.user_data   = "$itor";
      tf_data.tfname      = tf_data.user_data;
      tf_data.sizetf      = 0;
      tf_data.compiletf   = sys_convert_compiletf;
      tf_data.calltf      = sys_itor_calltf;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiSizedFunc;
      tf_data.user_data   = "$realtobits";
      tf_data.tfname      = tf_data.user_data;
      tf_data.sizetf      = sizetf_64;
      tf_data.compiletf   = sys_convert_compiletf;
      tf_data.calltf      = sys_realtobits_calltf;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiSizedFunc;
      tf_data.user_data   = "$shortrealtobits";
      tf_data.tfname      = tf_data.user_data;
      tf_data.sizetf      = sizetf_32;
      tf_data.compiletf   = sys_convert_compiletf;
      tf_data.calltf      = sys_shortrealtobits_calltf;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.user_data   = "$rtoi";
      tf_data.tfname      = tf_data.user_data;
      tf_data.sizetf      = 0;
      tf_data.compiletf   = sys_convert_compiletf;
      tf_data.calltf      = sys_rtoi_calltf;
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
