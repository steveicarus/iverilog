/*
 * Copyright (c) 2003-2010 Michael Ruff (mruff  at chiaro.com)
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

# include  "vpi_config.h"
# include  "vpi_user.h"
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
      bits[0] = conv.bval[7]
	      | (conv.bval[6] << 8)
	      | (conv.bval[5] <<16)
	      | (conv.bval[4] <<24);
      bits[1] = conv.bval[3]
	      | (conv.bval[2] << 8)
	      | (conv.bval[1] <<16)
	      | (conv.bval[0] <<24);
#else
      bits[0] = conv.bval[0]
	      | (conv.bval[1] << 8)
	      | (conv.bval[2] <<16)
	      | (conv.bval[3] <<24);
      bits[1] = conv.bval[4]
	      | (conv.bval[5] << 8)
	      | (conv.bval[6] <<16)
	      | (conv.bval[7] <<24);
#endif
}

static PLI_INT32 sizetf_32 (char*x) { return 32; }
static PLI_INT32 sizetf_64 (char*x) { return 64; }

static PLI_INT32 sys_convert_compiletf(char *name)
{
    vpiHandle call_hand, argv, arg;
    int rtn = 0;

    call_hand = vpi_handle(vpiSysTfCall, 0);
    argv = vpi_iterate(vpiArgument, call_hand);
    arg  = vpi_scan(argv);

    if (!argv) {
	vpi_printf("ERROR: %s requires a parameter.\n",
		   vpi_get_str(vpiName, call_hand));
	rtn = -1;
    }

    if (!strcmp("$bitstoreal", name) && vpi_get(vpiSize, arg) != 64) {
	vpi_printf("ERROR: %s requires 64-bit argument.\n",
		   vpi_get_str(vpiName, call_hand));
	rtn = -1;
    }

    /* free iterator */
    vpi_free_object(argv);

    return rtn;
}

static PLI_INT32 sys_bitstoreal_calltf(char *user)
{
    vpiHandle sys, argv, arg;
    s_vpi_value value;

    PLI_UINT32 bits[2];

    /* find argument handle */
    sys  = vpi_handle(vpiSysTfCall, 0);
    argv = vpi_iterate(vpiArgument, sys);
    arg  = vpi_scan(argv);

    /* get value */
    value.format = vpiVectorVal;
    vpi_get_value(arg, &value);

    /* convert */
    bits[0] = (value.value.vector[0]).aval;
    bits[1] = (value.value.vector[1]).aval;
    value.value.real = bits2double(bits);
    value.format = vpiRealVal;

    /* return converted value */
    vpi_put_value(sys, &value, 0, vpiNoDelay);

    /* free iterator */
    vpi_free_object(argv);

    return 0;
}

static PLI_INT32 sys_itor_calltf(char *user)
{
    vpiHandle sys, argv, arg;
    s_vpi_value value;

    /* find argument handle */
    sys  = vpi_handle(vpiSysTfCall, 0);
    argv = vpi_iterate(vpiArgument, sys);
    arg  = vpi_scan(argv);

    /* get value */
    value.format = vpiIntVal;
    vpi_get_value(arg, &value);

    /* convert */
    value.value.real = (PLI_INT32)value.value.integer;
    value.format = vpiRealVal;

    /* return converted value */
    vpi_put_value(sys, &value, 0, vpiNoDelay);

    /* free iterator */
    vpi_free_object(argv);

    return 0;
}

static PLI_INT32 sys_realtobits_calltf(char *user)
{
    vpiHandle sys, argv, arg;
    s_vpi_value value;
    static struct t_vpi_vecval res[2];

    PLI_UINT32 bits[2];

    /* find argument handle */
    sys  = vpi_handle(vpiSysTfCall, 0);
    argv = vpi_iterate(vpiArgument, sys);
    arg  = vpi_scan(argv);

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
    vpi_put_value(sys, &value, 0, vpiNoDelay);

    /* free iterator */
    vpi_free_object(argv);

    return 0;
}

static PLI_INT32 sys_rtoi_calltf(char *user)
{
    vpiHandle sys, argv, arg;
    s_vpi_value value;
    static struct t_vpi_vecval res;

    /* find argument handle */
    sys  = vpi_handle(vpiSysTfCall, 0);
    argv = vpi_iterate(vpiArgument, sys);
    arg  = vpi_scan(argv);

    /* get value */
    value.format = vpiRealVal;
    vpi_get_value(arg, &value);

    /* convert */
    res.aval = (unsigned)value.value.real;
    res.bval = 0;

    value.format = vpiVectorVal;
    value.value.vector = &res;

    /* return converted value */
    vpi_put_value(sys, &value, 0, vpiNoDelay);

    /* free iterator */
    vpi_free_object(argv);

    return 0;
}

void sys_convert_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysFunc;
      tf_data.user_data = "$bitstoreal";
      tf_data.tfname    = tf_data.user_data;
      tf_data.sizetf    = sizetf_64;
      tf_data.compiletf = sys_convert_compiletf;
      tf_data.calltf    = sys_bitstoreal_calltf;
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysFunc;
      tf_data.user_data = "$itor";
      tf_data.tfname    = tf_data.user_data;
      tf_data.sizetf    = sizetf_64;
      tf_data.compiletf = sys_convert_compiletf;
      tf_data.calltf    = sys_itor_calltf;
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysFunc;
      tf_data.user_data = "$realtobits";
      tf_data.tfname    = tf_data.user_data;
      tf_data.sizetf    = sizetf_64;
      tf_data.compiletf = sys_convert_compiletf;
      tf_data.calltf    = sys_realtobits_calltf;
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysFunc;
      tf_data.user_data = "$rtoi";
      tf_data.tfname    = tf_data.user_data;
      tf_data.sizetf    = sizetf_32;
      tf_data.compiletf = sys_convert_compiletf;
      tf_data.calltf    = sys_rtoi_calltf;
      vpi_register_systf(&tf_data);
}
