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
#ident "$Id: sys_display.c,v 1.1 1999/08/15 01:23:56 steve Exp $"
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

static int sys_display_calltf(char *xx)
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

      vpi_printf("\n");

      vpi_free_object(argv);
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
      vpi_register_systf(&tf_data);
}


/*
 * $Log: sys_display.c,v $
 * Revision 1.1  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 */

