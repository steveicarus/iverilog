/*
 * Copyright (c) 2003 Michael Ruff (mruff  at chiaro.com)
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
#ident "$Id: sys_convert.c,v 1.1 2003/03/07 02:44:34 steve Exp $"
#endif

# include  "vpi_user.h"
# include  "config.h"
# include  <stdio.h>
# include  <math.h>

/*
 */
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


static int sizetf_64 (char*x) { return 64; }

static int sys_real2bits_compiletf(char *name)
{
    vpiHandle call_hand, argv;

    call_hand = vpi_handle(vpiSysTfCall, 0);
    argv = vpi_iterate(vpiArgument, call_hand);

    if (!argv) {
	vpi_printf("ERROR: %s requires a parameter.\n",
		   vpi_get_str(vpiName, call_hand));
	return -1;
    }

    return 0;
}

static int sys_real2bits_calltf(char *user)
{
    vpiHandle sys, argv, arg;
    s_vpi_value value;
    static struct t_vpi_vecval res[2];

    PLI_UINT32 bits[2];

    /* find argument handle */
    sys  = vpi_handle(vpiSysTfCall, 0);
    argv = vpi_iterate(vpiArgument, sys);
    arg  = vpi_scan(argv);

    /* get current value in desired destination format */
    value.format = vpiRealVal;
    vpi_get_value(arg, &value);

    double2bits(value.value.real, bits);

    res[0].aval = bits[0];
    res[0].bval = 0;
    res[1].aval = bits[1];
    res[1].bval = 0;

    value.format = vpiVectorVal;
    value.value.vector = res;

    /* return converted value */
    vpi_put_value(sys, &value, 0, vpiNoDelay);

    return 0;
}

void sys_convert_register()
{
      s_vpi_systf_data tf_data;
#if 0
      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$bitstoreal";
      tf_data.calltf    = sys_convert_calltf;
      tf_data.compiletf = sys_convert_compiletf;
      tf_data.sizetf    = sizetf_64;
      tf_data.user_data = "0$bitstoreal";
      vpi_register_systf(&tf_data);
#endif

#if 0
      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$itor";
      tf_data.calltf    = sys_convert_calltf;
      tf_data.compiletf = sys_convert_compiletf;
      tf_data.sizetf    = sizetf_64;
      tf_data.user_data = "1$itor";
      vpi_register_systf(&tf_data);
#endif

      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$realtobits";
      tf_data.calltf    = sys_real2bits_calltf;
      tf_data.compiletf = sys_real2bits_compiletf;
      tf_data.sizetf    = sizetf_64;
      tf_data.user_data = "$realtobits";
      vpi_register_systf(&tf_data);

#if 0
      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$rtoi";
      tf_data.calltf    = sys_convert_calltf;
      tf_data.compiletf = sys_convert_compiletf;
      tf_data.sizetf    = sizetf_32;
      tf_data.user_data = "3$rtoi";
      vpi_register_systf(&tf_data);
#endif
}

/*
 * $Log: sys_convert.c,v $
 * Revision 1.1  2003/03/07 02:44:34  steve
 *  Implement $realtobits.
 *
 */
