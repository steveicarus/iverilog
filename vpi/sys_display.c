/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: sys_display.c,v 1.7 1999/11/06 22:16:50 steve Exp $"
#endif

# include  "vpi_user.h"
# include  <assert.h>
# include  <string.h>
# include  <ctype.h>
# include  <stdlib.h>

static void format_binary(vpiHandle argv, int fsize)
{
      s_vpi_value value;
      vpiHandle item = vpi_scan(argv);
      if (item == 0) return;

      value.format = vpiBinStrVal;
      vpi_get_value(item, &value);
      vpi_printf("%s", value.value.str);
}

static void format_octal(vpiHandle argv, int fsize)
{
      s_vpi_value value;
      vpiHandle item = vpi_scan(argv);
      if (item == 0) return;

      value.format = vpiOctStrVal;
      vpi_get_value(item, &value);
      vpi_printf("%s", value.value.str);
}

static void format_decimal(vpiHandle argv, int fsize)
{
      s_vpi_value value;
      vpiHandle item = vpi_scan(argv);
      if (item == 0) return;

      value.format = vpiDecStrVal;
      vpi_get_value(item, &value);
      vpi_printf("%s", value.value.str);
}

static void format_hex(vpiHandle argv, int fsize)
{
      s_vpi_value value;
      vpiHandle item = vpi_scan(argv);
      if (item == 0) return;

      value.format = vpiHexStrVal;
      vpi_get_value(item, &value);
      vpi_printf("%s", value.value.str);
}

static void format_m(vpiHandle argv, int fsize)
{
      vpiHandle item = vpi_scan(argv);
      if (item == 0) return;

      vpi_printf("%s", vpi_get_str(vpiFullName, item));
}

static void format_time(vpiHandle argv, int fsize)
{
      s_vpi_value value;
      vpiHandle item = vpi_scan(argv);
      if (item == 0) return;

      value.format = vpiDecStrVal;
      vpi_get_value(item, &value);
      vpi_printf("%s", value.value.str);
}

/*
 * If $display discovers a string as a parameter, this function is
 * called to process it as a format string. I need the argv handle as
 * well so that I can look for arguments as I move forward through the
 * string.
 */
static void format(s_vpi_value*fmt, vpiHandle argv)
{
      char buf[256];
      char*cp = fmt->value.str;
      assert(fmt->value.str);

      while (*cp) {
	    size_t cnt = strcspn(cp, "%\\");
	    if (cnt > 0) {
		  if (cnt >= sizeof buf)
			cnt = sizeof buf - 1;
		  strncpy(buf, cp, cnt);
		  buf[cnt] = 0;
		  vpi_printf("%s", buf);
		  cp += cnt;

	    } else if (*cp == '%') {
		  int fsize = -1;

		  cp += 1;
		  if (isdigit(*cp))
			fsize = strtoul(cp, &cp, 10);

		  switch (*cp) {
		      case 0:
			break;
		      case 'b':
		      case 'B':
			format_binary(argv, fsize);
			cp += 1;
			break;
		      case 'd':
		      case 'D':
			format_decimal(argv, fsize);
			cp += 1;
			break;
		      case 'h':
		      case 'H':
		      case 'x':
		      case 'X':
			format_hex(argv, fsize);
			cp += 1;
			break;
		      case 'm':
			format_m(argv, fsize);
			cp += 1;
			break;
		      case 'o':
		      case 'O':
			format_octal(argv, fsize);
			cp += 1;
			break;
		      case 't':
		      case 'T':
			format_time(argv, fsize);
			cp += 1;
			break;
		      case '%':
			vpi_printf("%%");
			cp += 1;
			break;
		      default:
			vpi_printf("%c", *cp);
			cp += 1;
			break;
		  }

	    } else {

		  cp += 1;
		  switch (*cp) {
		      case 0:
			break;
		      case 'n':
			vpi_printf("\n");
			cp += 1;
			break;
		      default:
			vpi_printf("%c", *cp);
			cp += 1;
		  }
	    }
      }
}

static int sys_display_calltf(char *name)
{
      s_vpi_value value;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);

      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item;

      for (item = vpi_scan(argv) ;  item ;  item = vpi_scan(argv)) {

	    switch (vpi_get(vpiType, item)) {

		case 0:
		  vpi_printf(" ");
		  break;

		case vpiConstant:
		  value.format = vpiObjTypeVal;
		  vpi_get_value(item, &value);
		  switch (value.format) {
		      case vpiStringVal:
			format(&value, argv);
			break;
		      case vpiSuppressVal:
			break;
		      default:
			vpi_printf("?");
			break;
		  }
		  break;

		case vpiNet:
		case vpiReg:
		  value.format = vpiBinStrVal;
		  vpi_get_value(item, &value);
		  vpi_printf("%s", value.value.str);
		  break;

		case vpiTimeVar:
		  value.format = vpiTimeVal;
		  vpi_get_value(item, &value);
		  vpi_printf("%u", value.value.time->low);
		  break;

		default:
		  vpi_printf("?");
		  break;
	    }
      }

      if (strcmp(name,"$display") == 0)
	    vpi_printf("\n");

      return 0;
}


static int format_str(char*fmt, int argc, vpiHandle*argv)
{
      s_vpi_value value;
      char buf[256];
      char*cp = fmt;
      int idx;

      assert(fmt);

      idx = 0;

      while (*cp) {
	    size_t cnt = strcspn(cp, "%\\");
	    if (cnt > 0) {
		  if (cnt >= sizeof buf)
			cnt = sizeof buf - 1;
		  strncpy(buf, cp, cnt);
		  buf[cnt] = 0;
		  vpi_printf("%s", buf);
		  cp += cnt;

	    } else if (*cp == '%') {
		  int fsize = -1;

		  cp += 1;
		  if (isdigit(*cp))
			fsize = strtoul(cp, &cp, 10);

		  switch (*cp) {
		      case 0:
			break;
		      case 'b':
		      case 'B':
			value.format = vpiBinStrVal;
			vpi_get_value(argv[idx++], &value);
			vpi_printf("%s", value.value.str);
			cp += 1;
			break;
		      case 'd':
		      case 'D':
			value.format = vpiDecStrVal;
			vpi_get_value(argv[idx++], &value);
			vpi_printf("%s", value.value.str);
			cp += 1;
			break;
		      case 'h':
		      case 'H':
		      case 'x':
		      case 'X':
			value.format = vpiHexStrVal;
			vpi_get_value(argv[idx++], &value);
			vpi_printf("%s", value.value.str);
			cp += 1;
			break;
		      case 'm':
			vpi_printf("%s",
				   vpi_get_str(vpiFullName, argv[idx++]));
			cp += 1;
			break;
		      case 'o':
		      case 'O':
			value.format = vpiOctStrVal;
			vpi_get_value(argv[idx++], &value);
			vpi_printf("%s", value.value.str);
			cp += 1;
			break;
		      case 't':
		      case 'T':
			value.format = vpiDecStrVal;
			vpi_get_value(argv[idx++], &value);
			vpi_printf("%s", value.value.str);
			cp += 1;
			break;
		      case '%':
			vpi_printf("%%");
			cp += 1;
			break;
		      default:
			vpi_printf("%c", *cp);
			cp += 1;
			break;
		  }

	    } else {

		  cp += 1;
		  switch (*cp) {
		      case 0:
			break;
		      case 'n':
			vpi_printf("\n");
			cp += 1;
			break;
		      default:
			vpi_printf("%c", *cp);
			cp += 1;
		  }
	    }
      }

      return idx;
}

/*
 * The strobe implementation takes the parameter handles that are
 * passed to the calltf and puts them in to an array for safe
 * keeping. That array (and other bookkeeping) is passed, via the
 * struct_cb_info object, to the REadOnlySych function strobe_cb,
 * where it is use to perform the actual formatting and printing.
 */
struct strobe_cb_info {
      char*name;
      vpiHandle*items;
      unsigned nitems;
};

static int strobe_cb(p_cb_data cb)
{
      s_vpi_value value;
      unsigned idx;
      struct strobe_cb_info*info = (struct strobe_cb_info*)cb->user_data;


      for (idx = 0 ;  idx < info->nitems ;  idx += 1) {
	    vpiHandle item = info->items[idx];

	    switch (vpi_get(vpiType, item)) {

		case 0:
		  vpi_printf(" ");
		  break;

		case vpiConstant:
		  if (vpi_get(vpiConstType, item) == vpiStringConst) {
			value.format = vpiStringVal;
			vpi_get_value(item, &value);
			idx += format_str(value.value.str,
					  info->nitems-idx-1,
					  info->items+1);
		  } else {
			value.format = vpiBinStrVal;
			vpi_get_value(item, &value);
			vpi_printf("%s", value.value.str);
		  }
		  break;

		case vpiNet:
		case vpiReg:
		  value.format = vpiBinStrVal;
		  vpi_get_value(item, &value);
		  vpi_printf("%s", value.value.str);
		  break;

		case vpiTimeVar:
		  value.format = vpiTimeVal;
		  vpi_get_value(item, &value);
		  vpi_printf("%u", value.value.time->low);
		  break;

		default:
		  vpi_printf("?");
		  break;
	    }
      }

      vpi_printf("\n");

      free(info->name);
      free(info->items);
      free(info);

      return 0;
}

static int sys_strobe_calltf(char*name)
{
      struct t_cb_data cb;
      struct t_vpi_time time;
      struct strobe_cb_info*info;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);

      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item;

      vpiHandle*items;
      unsigned nitems = 1;
      items = malloc(sizeof(vpiHandle));
      items[0] = vpi_scan(argv);
      for (item = vpi_scan(argv) ;  item ;  item = vpi_scan(argv)) {
	    items = realloc(items, (nitems+1)*sizeof(vpiHandle));
	    items[nitems] = item;
	    nitems += 1;
      }

      info = calloc(1, sizeof(struct strobe_cb_info));
      info->name = strdup(name);
      info->items = items;
      info->nitems = nitems;

      time.type = vpiSimTime;
      time.low = 0;
      time.high = 0;

      cb.reason = cbReadOnlySynch;
      cb.cb_rtn = strobe_cb;
      cb.time = &time;
      cb.obj = 0;
      cb.user_data = (char*)info;
      vpi_register_cb(&cb);
      return 0;
}

static int sys_monitor_calltf(char*name)
{
      struct t_cb_data cb;
      struct t_vpi_time time;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item;

      time.type = vpiSuppressTime;

      for (item = vpi_scan(argv) ;  item ;  item = vpi_scan(argv)) {

	    switch (vpi_get(vpiType, item)) {

		default:
		  break;
	    }

      }

      return 0;
}

void sys_display_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$display";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$display";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$write";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$write";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$strobe";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$strobe";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitor";
      tf_data.calltf    = sys_monitor_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitor";
      vpi_register_systf(&tf_data);
}


/*
 * $Log: sys_display.c,v $
 * Revision 1.7  1999/11/06 22:16:50  steve
 *  Get the $strobe task working.
 *
 * Revision 1.6  1999/10/29 03:37:22  steve
 *  Support vpiValueChance callbacks.
 *
 * Revision 1.5  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 * Revision 1.4  1999/10/10 14:50:50  steve
 *  Add Octal dump format.
 *
 * Revision 1.3  1999/10/08 17:47:49  steve
 *  Add the %t formatting escape.
 *
 * Revision 1.2  1999/09/29 01:41:18  steve
 *  Support the $write system task, and have the
 *  vpi_scan function free iterators as needed.
 *
 * Revision 1.1  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 */

