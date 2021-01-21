/*
 * Copyright (c) 2002-2021 Stephen Williams (steve@icarus.com)
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
# include  <string.h>
# include  <stdlib.h>
# include  <assert.h>

/*
 * Compare the +arguments passed to the simulator with the argument
 * passed to the $test$plusargs. If there is a simulator argument that
 * is like this argument, then return true. Otherwise return false.
 */
static PLI_INT32 sys_test_plusargs_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      s_vpi_value val;
      s_vpi_vlog_info info;
      int idx;
      int flag = 0;
      size_t slen, len;

      (void)name; /* Parameter is not used. */

      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      val.format = vpiStringVal;
      vpi_get_value(vpi_scan(argv), &val);
      slen = strlen(val.value.str);

      vpi_get_vlog_info(&info);

	/* Look for a +arg that matches the prefix supplied. */
      for (idx = 0 ;  idx < info.argc ;  idx += 1) {

	      /* Skip arguments that are not +args. */
	    if (info.argv[idx][0] != '+')
		  continue;

	    len = strlen(info.argv[idx]+1);
	    if (len < slen)
		  continue;

	    if (strncmp(val.value.str, info.argv[idx]+1, slen) != 0)
		  continue;

	    flag = 1;
	    break;
      }

      val.format = vpiIntVal;
      val.value.integer = flag;
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_value_plusargs_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Check that the first argument is a string. */
      arg = vpi_scan(argv);
      assert(arg != 0);
      if ( ! is_string_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument must be a string.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's requires a second variable argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      switch (vpi_get(vpiType, arg)) {

	  case vpiReg:
	  case vpiIntegerVar:
	  case vpiBitVar:
	  case vpiByteVar:
	  case vpiShortIntVar:
	  case vpiIntVar:
	  case vpiLongIntVar:
	  case vpiRealVar:
	  case vpiTimeVar:
	  case vpiStringVar:
	    break;

	  default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's second argument must be a variable, found a %s.\n",
		       name, vpi_get_str(vpiType, arg));
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "two arguments", 0);

      return 0;
}

static PLI_INT32 sys_value_plusargs_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      s_vpi_vlog_info info;
      s_vpi_value fmt;
      s_vpi_value res;
      char msg[64];
      char*cp;
      int idx;
      int flag = 0;
      size_t slen, len;

      vpiHandle callh  = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      fmt.format = vpiStringVal;
      vpi_get_value(vpi_scan(argv), &fmt);

	/* Check for the start of a format string. */
      cp = strchr(fmt.value.str, '%');
      if (cp == 0) {
	    snprintf(msg, sizeof(msg), "ERROR: %s:%d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));
	    msg[sizeof(msg)-1] = 0;

	    vpi_printf("%s %s is missing a format code.\n", msg, name);
	    vpi_printf("%*s \"%s\".\n", (int)strlen(msg), " ", fmt.value.str);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* This is the length of string we will look for. */
      slen = cp - fmt.value.str;

	/* Skip a zero. */
      cp += 1;
      if (*cp == '0') cp += 1;

	/* Check the format code. */
      switch (*cp) {
	  case 'd':
	  case 'D':
	  case 'o':
	  case 'O':
	  case 'h':
	  case 'H':
	  case 'x':
	  case 'X':
	  case 'b':
	  case 'B':
	  case 'e':
	  case 'E':
	  case 'f':
	  case 'F':
	  case 'g':
	  case 'G':
	  case 's':
	  case 'S':
	    break;
	  default:
	    snprintf(msg, sizeof(msg), "ERROR: %s:%d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));
	    msg[sizeof(msg)-1] = 0;

	    vpi_printf("%s %s has an invalid format string:\n", msg, name);
	    vpi_printf("%*s \"%s\".\n", (int)strlen(msg), " ", fmt.value.str);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Warn if there is any trailing garbage. */
      if (*(cp+1) != '\0') {
	    snprintf(msg, sizeof(msg), "WARNING: %s:%d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));
	    msg[sizeof(msg)-1] = 0;

	    vpi_printf("%s Skipping trailing garbage in %s's format string:\n",
	               msg, name);
	    vpi_printf("%*s \"%s\".\n", (int)strlen(msg), " ", fmt.value.str);
	    *(cp+1) = '\0';
      }

      vpi_get_vlog_info(&info);

	/* Look for a +arg that matches the prefix supplied. */
      for (idx = 0 ;  idx < info.argc ;  idx += 1) {
	    char*sp, *tp, *end;
            size_t sp_len;

	      /* Skip arguments that are not +args. */
	    if (info.argv[idx][0] != '+')
		  continue;

	    len = strlen(info.argv[idx]+1);
	    if (len < slen)
		  continue;

	    if (strncmp(fmt.value.str, info.argv[idx]+1, slen) != 0)
		  continue;

	    sp = info.argv[idx]+1+slen;
            sp_len = strlen(sp);
	    switch (*cp) {
		case 'd':
		case 'D':
		  res.format = vpiDecStrVal;
		    /* A decimal string can set the value to "x" or "z". */
		  if (sp_len == strspn(sp, "xX_") ||
		      sp_len == strspn(sp, "zZ_")) {
			res.value.str = sp;
		    /* A decimal string must contain only these characters.
		     * A decimal string can not start with an "_" character.
		     * A "-" can only be at the start of the string. */
		  } else if (sp_len != strspn(sp, "-0123456789_") ||
		             *sp == '_' ||
		             ((tp = strrchr(sp, '-')) && tp != sp)) {
			res.value.str = "x";
			snprintf(msg, sizeof(msg), "WARNING: %s:%d:",
			         vpi_get_str(vpiFile, callh),
			         (int)vpi_get(vpiLineNo, callh));
			msg[sizeof(msg)-1] = 0;
			vpi_printf("%s Invalid decimal value passed to %s:\n",
			           msg, name);
			vpi_printf("%*s \"%s\".\n", (int)strlen(msg), " ", sp);
		  } else {
			res.value.str = sp;
		  }
		  break;
		case 'o':
		case 'O':
		  res.format = vpiOctStrVal;
		    /* An octal string must contain only these characters.
		     * An octal string can not start with an "_" character.
		     * A "-" can only be at the start of the string. */
		  if (sp_len != strspn(sp, "-01234567_xXzZ") ||
		      *sp == '_' || ((tp = strrchr(sp, '-')) && tp != sp)) {
			res.value.str = "x";
			snprintf(msg, sizeof(msg), "WARNING: %s:%d:",
			         vpi_get_str(vpiFile, callh),
			         (int)vpi_get(vpiLineNo, callh));
			msg[sizeof(msg)-1] = 0;
			vpi_printf("%s Invalid octal value passed to %s:\n",
			           msg, name);
			vpi_printf("%*s \"%s\".\n", (int)strlen(msg), " ", sp);
		  } else {
			res.value.str = sp;
		  }
		  break;
		case 'h':
		case 'H':
		case 'x':
		case 'X':
		  res.format = vpiHexStrVal;
		    /* A hex. string must contain only these characters.
		     * A hex. string can not start with an "_" character.
		     * A "-" can only be at the start of the string. */
		  if (sp_len != strspn(sp, "-0123456789aAbBcCdDeEfF_xXzZ") ||
		      *sp == '_' || ((tp = strrchr(sp, '-')) && tp != sp)) {
			res.value.str = "x";
			snprintf(msg, sizeof(msg), "WARNING: %s:%d:",
			         vpi_get_str(vpiFile, callh),
			         (int)vpi_get(vpiLineNo, callh));
			msg[sizeof(msg)-1] = 0;
			vpi_printf("%s Invalid hex value passed to %s:\n",
			           msg, name);
			vpi_printf("%*s \"%s\".\n", (int)strlen(msg), " ", sp);
		  } else {
			res.value.str = sp;
		  }
		  break;
		case 'b':
		case 'B':
		  res.format = vpiBinStrVal;
		    /* A binary string must contain only these characters.
		     * A binary string can not start with an "_" character.
		     * A "-" can only be at the start of the string. */
		  if (sp_len != strspn(sp, "-01_xXzZ") ||
		      *sp == '_' || ((tp = strrchr(sp, '-')) && tp != sp)) {
			res.value.str = "x";
			snprintf(msg, sizeof(msg), "WARNING: %s:%d:",
			         vpi_get_str(vpiFile, callh),
			         (int)vpi_get(vpiLineNo, callh));
			msg[sizeof(msg)-1] = 0;
			vpi_printf("%s Invalid binary value passed to %s:\n",
			           msg, name);
			vpi_printf("%*s \"%s\".\n", (int)strlen(msg), " ", sp);
		  } else {
			res.value.str = sp;
		  }
		  break;
		case 'e':
		case 'E':
		case 'f':
		case 'F':
		case 'g':
		case 'G':
		  res.format = vpiRealVal;
		  res.value.real = strtod(sp, &end);
		    /* If we didn't get a full conversion print a warning. */
		  if (*end) {
			  /* We had an invalid value passed. */
			if (end == sp) {
			      snprintf(msg, sizeof(msg), "WARNING: %s:%d:",
			               vpi_get_str(vpiFile, callh),
			               (int)vpi_get(vpiLineNo, callh));
			      msg[sizeof(msg)-1] = 0;
			      vpi_printf("%s Invalid real value passed to "
			                 "%s:\n", msg, name);
			      vpi_printf("%*s \"%s\".\n", (int)strlen(msg), " ",
			                 sp);
			  /* We have extra garbage at the end. */
			} else {
			      snprintf(msg, sizeof(msg), "WARNING: %s:%d:",
			               vpi_get_str(vpiFile, callh),
			               (int)vpi_get(vpiLineNo, callh));
			      msg[sizeof(msg)-1] = 0;
			      vpi_printf("%s Extra character(s) \"%s\" found "
			                 "in %s's real string:\n",
			                 msg, end, name);
			      vpi_printf("%*s \"%s\".\n", (int)strlen(msg), " ",
			                 sp);
			}
		  }
		  break;
		case 's':
		case 'S':
		  res.format = vpiStringVal;
		  res.value.str = sp;
		  break;
		default:
		  assert(0);
	    }

	    vpi_put_value(vpi_scan(argv), &res, 0, vpiNoDelay);
	    flag = 1;
	    break;
      }

      res.format = vpiIntVal;
      res.value.integer = flag;
      vpi_put_value(callh, &res, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

void sys_plusargs_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname      = "$test$plusargs";
      tf_data.calltf      = sys_test_plusargs_calltf;
      tf_data.compiletf   = sys_one_string_arg_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$test$plusargs";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname      = "$value$plusargs";
      tf_data.calltf      = sys_value_plusargs_calltf;
      tf_data.compiletf   = sys_value_plusargs_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$value$plusargs";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

}
