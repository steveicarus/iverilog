/*
 * Copyright (c) 2002-2010 Stephen Williams (steve@icarus.com)
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

# include  <vpi_user.h>
# include  <string.h>
# include  <stdlib.h>
# include  <assert.h>

static PLI_INT32 sys_plusargs_sizetf(char*x)
{
      return 32;
}

/*
 * The compiletf for $test$plusargs checks that there is one argument
 * to the function call, and that argument is a constant string.
 */
static PLI_INT32 sys_test_plusargs_compiletf(char*xx)
{
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg;

      if (argv == 0) {
	    vpi_printf("ERROR: $test$plusargs requires one argument\n");
	    vpi_sim_control(vpiFinish, 1);
	    return 0;
      }

      arg = vpi_scan(argv);
      assert(arg != 0);

      switch (vpi_get(vpiType, arg)) {
	  case vpiConstant:
	    if (vpi_get(vpiConstType, arg) != vpiStringConst) {
		  vpi_printf("ERROR: Argument of $test$plusargs "
			     " must be a constant string.\n");
		  vpi_sim_control(vpiFinish, 1);
		  return 0;
	    }
	    break;

	  default:
	    vpi_printf("ERROR: Argument of $test$plusargs "
		       " must be a constant string.\n");
	    vpi_sim_control(vpiFinish, 1);
	    return 0;
      }


      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_printf("ERROR: too many arguments to $test$plusargs\n");
	    vpi_sim_control(vpiFinish, 1);
      }

      return 0;
}

/*
 * Compare the +arguments passed to the simulator with the argument
 * passed to the $test$plusargs. If there is a simulator argument that
 * is like this argument, then return true. Otherwise return false.
 */
static PLI_INT32 sys_test_plusargs_calltf(char*xx)
{
      int idx;
      int flag = 0;
      size_t slen, len;
      s_vpi_vlog_info info;
      s_vpi_value value;
      s_vpi_value result;

      vpiHandle sys  = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg  = vpi_scan(argv);

      value.format = vpiStringVal;
      vpi_get_value(arg, &value);
      slen = strlen(value.value.str);

      vpi_get_vlog_info(&info);

	/* Look for a +arg that matches the prefix supplied. */
      for (idx = 0 ;  idx < info.argc ;  idx += 1) {

	      /* Skip arguments that are not +args. */
	    if (info.argv[idx][0] != '+')
		  continue;

	    len = strlen(info.argv[idx]+1);
	    if (len < slen)
		  continue;

	    if (strncmp(value.value.str, info.argv[idx]+1, slen) != 0)
		  continue;

	    flag = 1;
	    break;
      }

      result.format = vpiIntVal;
      result.value.integer = flag;
      vpi_put_value(sys, &result, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 sys_value_plusargs_compiletf(char*xx)
{
      s_vpi_value value;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg;

      if (argv == 0) {
	    vpi_printf("ERROR: $value$plusargs requires two arguments\n");
	    vpi_sim_control(vpiFinish, 1);
	    return 0;
      }

      arg = vpi_scan(argv);
      assert(arg != 0);

      switch (vpi_get(vpiType, arg)) {
	  case vpiConstant:
	    if (vpi_get(vpiConstType, arg) != vpiStringConst) {
		  vpi_printf("ERROR: First argument of $value$plusargs "
			     " must be a constant string.\n");
		  vpi_sim_control(vpiFinish, 1);
		  return 0;
	    }
	    break;

	  default:
	    vpi_printf("ERROR: First argument of $value$plusargs "
		       " must be a constant string.\n");
	    vpi_sim_control(vpiFinish, 1);
	    return 0;
	    break;
      }

	/* Check that the format string has a reasonable format. */
      value.format = vpiStringVal;
      vpi_get_value(arg, &value);
      { char*fmt = value.value.str;
        char*cp = strchr(fmt, '%');

	if (cp == 0) {
	      vpi_printf("ERROR: Invalid argument format string"
			 ": %s\n", fmt);
	      vpi_sim_control(vpiFinish, 1);
	      return 0;
	}

	cp += 1;
	if (*cp == '0')
	      cp += 1;

	switch (*cp) {
	    case 'd':
	    case 'o':
	    case 'b':
	    case 'h':
	    case 's':
	      cp += 1;
	      break;
	    default:
	      vpi_printf("ERROR: Invalid argument format string"
			 ": %s\n", fmt);
	      vpi_sim_control(vpiFinish, 1);
	      return 0;
	}

	if (*cp != 0) {
	      vpi_printf("ERROR: Trailing junk after value format"
			 ": %s\n", fmt);
	      vpi_sim_control(vpiFinish, 1);
	      return 0;
	}
      }

      arg = vpi_scan(argv);
      if (argv == 0) {
	    vpi_printf("ERROR: $value$plusargs requires two arguments\n");
	    vpi_sim_control(vpiFinish, 1);
	    return 0;
      }

      switch (vpi_get(vpiType, arg)) {

	  case vpiReg:
	  case vpiIntegerVar:
	    break;

	  default:
	    vpi_printf("ERROR: value field doesn\'t match format: %s\n",
		       value.value.str);
	    vpi_sim_control(vpiFinish, 1);
	    return 0;
      }

      arg = vpi_scan(argv);
      if (arg != 0) {
	    vpi_printf("ERROR: too many arguments to $value$plusargs\n");
	    vpi_sim_control(vpiFinish, 1);
	    return 0;
      }

      return 0;
}

static PLI_INT32 sys_value_plusargs_calltf(char*xx)
{
      char*cp;
      int idx;
      int flag = 0;
      size_t slen, len;
      s_vpi_vlog_info info;
      s_vpi_value format;
      s_vpi_value result;

      vpiHandle sys  = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle arg1  = vpi_scan(argv);
      vpiHandle arg2  = vpi_scan(argv);

      format.format = vpiStringVal;
      vpi_get_value(arg1, &format);

      vpi_get_vlog_info(&info);

      cp = strchr(format.value.str, '%');
      assert(cp);
      slen = cp - format.value.str;

      cp += 1;
      if (*cp == '0')
	    cp += 1;

      for (idx = 0 ;  idx < info.argc ;  idx += 1) {

	    if (info.argv[idx][0] != '+')
		  continue;

	    len = strlen(info.argv[idx]+1);
	    if (len < slen)
		  continue;

	    if (strncmp(format.value.str, info.argv[idx]+1, slen) != 0)
		  continue;

	    switch (*cp) {
		case 'd':
		  result.format = vpiIntVal;
		  result.value.integer = strtoul(info.argv[idx]+1+slen,0,10);
		  break;
		case 'o':
		  result.format = vpiIntVal;
		  result.value.integer = strtoul(info.argv[idx]+1+slen,0,8);
		  break;
		case 'h':
		  result.format = vpiIntVal;
		  result.value.integer = strtoul(info.argv[idx]+1+slen,0,16);
		  break;
		case 'b':
		  result.format = vpiIntVal;
		  result.value.integer = strtoul(info.argv[idx]+1+slen,0,12);
		  break;
		case 's':
		  result.format = vpiStringVal;
		  result.value.str = info.argv[idx]+1+slen;
		  break;
		default:
		  assert(0);
	    }

	    vpi_put_value(arg2, &result, 0, vpiNoDelay);
	    flag = 1;
	    break;
      }

      result.format = vpiIntVal;
      result.value.integer = flag;
      vpi_put_value(sys, &result, 0, vpiNoDelay);

      return 0;
}

void sys_plusargs_register()
{
      s_vpi_systf_data tf_data;


      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$test$plusargs";
      tf_data.calltf    = sys_test_plusargs_calltf;
      tf_data.compiletf = sys_test_plusargs_compiletf;
      tf_data.sizetf    = sys_plusargs_sizetf;
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$value$plusargs";
      tf_data.calltf    = sys_value_plusargs_calltf;
      tf_data.compiletf = sys_value_plusargs_compiletf;
      tf_data.sizetf    = sys_plusargs_sizetf;
      vpi_register_systf(&tf_data);

}
