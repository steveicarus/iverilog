/*
 * Copyright (c) 2006-2009 Stephen Williams (steve@icarus.com)
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

  /* round() is ISO C99 from math.h. This define should enable it. */
# define _ISOC99_SOURCE 1
# define _SVID_SOURCE 1

# include  "vpi_user.h"
# include  "sys_priv.h"
# include  <ctype.h>
# include  <errno.h>
# include  <string.h>
# include  <stdio.h>
# include  <stdlib.h>
# include  <math.h>
# include  <assert.h>

struct byte_source {
      const char*str;
      FILE*fd;
};

static int byte_getc(struct byte_source*byte)
{
      int ch;
      if (byte->str) {
	    if (byte->str[0] == 0) return EOF;

	    return *(byte->str)++;
      }

      ch = fgetc(byte->fd);
      return ch;
}

static void byte_ungetc(struct byte_source*src, int ch)
{
      if (ch == EOF) return;

      if (src->str) {
	    src->str -= 1;
	    return;
      }

      assert(src->fd);
      ungetc(ch, src->fd);
}


/*
 * This function matches the input characters of a floating point
 * number and generates a floating point (double) from that string.
 *
 * Need support for +-Inf and NaN. Look at the $plusargs code.
 */
static double float_string(struct byte_source*src)
{
      int ch;
      char*str = 0;
      size_t len;
      double sign_flag = 1.0;

      ch = byte_getc(src);
	/* Skip leading space. */
      while (isspace(ch)) ch = byte_getc(src);

      if (ch == '+') {
	    sign_flag = 1.0;
	    ch = byte_getc(src);
      } else if (ch == '-') {
	    sign_flag = -1.0;
	    ch = byte_getc(src);
      }

      str = malloc(1);
      len = 0;
      *str = 0;

      while (isdigit(ch)) {
	    str = realloc(str, len+2);
	    str[len++] = ch;
	    ch = byte_getc(src);
      }

      if (ch == '.') {
	    str = realloc(str, len+2);
	    str[len++] = ch;
	    ch = byte_getc(src);
	    while (isdigit(ch)) {
		  str = realloc(str, len+2);
		  str[len++] = ch;
		  ch = byte_getc(src);
	    }
      }

      if (ch == 'e' || ch == 'E') {
	    str = realloc(str, len+2);
	    str[len++] = ch;
	    ch = byte_getc(src);

	    if (ch == '-' || ch == '+') {
		  str = realloc(str, len+2);
		  str[len++] = ch;
		  ch = byte_getc(src);
	    }

	    while (isdigit(ch)) {
		  src = realloc(str, len+2);
		  str[len++] = ch;
		  ch = byte_getc(src);
	    }
      }

      str[len] = 0;

      sign_flag *= strtod(str, 0);
      free(str);

      if (ch != EOF) byte_ungetc(src, ch);

      return sign_flag;
}

static int scan_format_float(struct byte_source*src,
			     vpiHandle arg)
{
      s_vpi_value val;

      val.format = vpiRealVal;
      val.value.real = float_string(src);
      vpi_put_value(arg, &val, 0, vpiNoDelay);

      return 1;
}

static int scan_format_float_time(vpiHandle callh,
				  struct byte_source*src,
				  vpiHandle arg)
{
      vpiHandle scope = vpi_handle(vpiScope, callh);
      int time_units = vpi_get(vpiTimeUnit, scope);
      double scale;

      s_vpi_value val;

      val.format = vpiRealVal;
      val.value.real = float_string(src);

	/* Round the value to the specified precision. Handle this bit
	   by shifting the decimal point to the precision where we
	   want to round, do the rounding, then shift the point back */
      scale = pow(10.0, timeformat_info.prec);
      val.value.real = round(val.value.real*scale) / scale;

	/* Change the units from the timeformat to the timescale. */
      scale = pow(10.0, timeformat_info.units - time_units);
      val.value.real *= scale;
      vpi_put_value(arg, &val, 0, vpiNoDelay);

      return 1;
}

/*
 * Match the %s format by loading a string of non-space characters.
 */
static int scan_format_string(struct byte_source*src, vpiHandle arg)
{
      char*tmp = malloc(1);
      size_t len = 0;
      s_vpi_value val;
      int ch;

      *tmp = 0;

      ch = byte_getc(src);
	/* Skip leading space. */
      while (isspace(ch)) ch = byte_getc(src);

      while (!isspace(ch)) {
	    if (ch == EOF) break;

	    tmp = realloc(tmp, len+2);
	    tmp[len++] = ch;

	    ch = byte_getc(src);
      }

      if (ch == EOF && len == 0) return 0;

      if (ch != EOF) byte_ungetc(src, ch);

      tmp[len] = 0;
      val.format = vpiStringVal;
      val.value.str = tmp;
      vpi_put_value(arg, &val, 0, vpiNoDelay);

      free(tmp);

      return 1;
}


/*
 * The $fscanf and $sscanf functions are the same except for the first
 * argument, which is the source. The wrapper functions below peel off
 * the first argument and make a byte_source object that then gets
 * passed to this function, which processes the rest of the function.
 */
static int scan_format(vpiHandle callh, struct byte_source*src, vpiHandle argv)
{
      s_vpi_value val;
      vpiHandle item;

      char*fmt, *fmtp;
      int rc = 0;
      int ch;

      int match_fail = 0;

	/* Get the format string. */
      item = vpi_scan(argv);
      assert(item);

      val.format = vpiStringVal;
      vpi_get_value(item, &val);
      fmtp = fmt = strdup(val.value.str);

	/* See if we are at EOF before we even start. */
      ch = byte_getc(src);
      if (ch == EOF) {
	    rc = EOF;
	    match_fail = 1;
      }
      byte_ungetc(src, ch);

      while ( fmtp && *fmtp != 0 && !match_fail) {

	    if (isspace(*fmtp)) {
		    /* White space matches a string of white space in
		       the input. The number of spaces is not
		       relevant, and the match may be 0 or more
		       spaces. */
		  while (*fmtp && isspace(*fmtp)) fmtp += 1;

		  ch = byte_getc(src);
		  while (isspace(ch)) ch = byte_getc(src);

		  if (ch != EOF) byte_ungetc(src, ch);

	    } else if (*fmtp != '%') {
		    /* Characters other than % match themselves. */
		  ch = byte_getc(src);
		  if (ch != *fmtp) {
			byte_ungetc(src, ch);
			break;
		  }

		  fmtp += 1;

	    } else {

		    /* We are at a pattern character. The pattern has
		       the format %<N>x no matter what the x code, so
		       parse it generically first. */

		  int suppress_flag = 0;
		  int length_field = -1;
		  int code = 0;
		  PLI_INT32 value;

		  char*tmp;

		  fmtp += 1;
		  if (*fmtp == '*') {
			suppress_flag = 1;
			fmtp += 1;
		  }
		  if (isdigit(*fmtp)) {
			length_field = 0;
			while (isdigit(*fmtp)) {
			      length_field *= 10;
			      length_field += *fmtp - '0';
			      fmtp += 1;
			}
		  }

		  code = *fmtp;
		  fmtp += 1;

		    /* The format string is parsed:
		       - length_field is the length,
		       - code is the format code character,
		       - suppress_flag is true if the length is an arg.
		       Now we interpret the code. */
		  switch (code) {

			  /* Read a '%' character from the input. */
		      case '%':
			ch = byte_getc(src);
			if (ch != '%') {
			      byte_ungetc(src, ch);
			      fmtp = 0;
			}
			break;

			/* Read a binary integer. If there is a match,
			   store that integer in the next argument and
			   increment the completion count. */
		      case 'b':
			  /* binary integer */
			tmp = malloc(2);
			value = 0;
			tmp[0] = 0;
			match_fail = 1;

			ch = byte_getc(src);
			  /* Skip leading space. */
			while (isspace(ch)) ch = byte_getc(src);

			while (strchr("01xXzZ?_", ch)) {
			      match_fail = 0;
			      if (ch == '?') ch = 'x';
			      if (ch != '_') {
				    ch = tolower(ch);
				    tmp[value++] = ch;
				    tmp = realloc(tmp, value+1);
				    tmp[value] = 0;
			      }
			      ch = byte_getc(src);
			}
			byte_ungetc(src, ch);

			if (match_fail) {
			      free(tmp);
			      break;
			}

			  /* Matched a binary value, put it to an argument. */
			item = vpi_scan(argv);
			assert(item);

			val.format = vpiBinStrVal;
			val.value.str = tmp;
			vpi_put_value(item, &val, 0, vpiNoDelay);

			free(tmp);
			rc += 1;
			break;

		      case 'c':
			ch = byte_getc(src);
			item = vpi_scan(argv);
			assert(item);

			val.format = vpiIntVal;
			val.value.integer = ch;
			vpi_put_value(item, &val, 0, vpiNoDelay);
			rc += 1;
			break;

		      case 'd':
			  /* decimal integer */
			tmp = malloc(2);
			value = 0;
			tmp[0] = 0;
			match_fail = 1;

			ch = byte_getc(src);
			  /* Skip leading space. */
			while (isspace(ch)) ch = byte_getc(src);

			while (isdigit(ch) || ch == '_' || (value == 0 && ch == '-')) {
			      match_fail = 0;
			      if (ch != '_') {
			            tmp[value++] = ch;
			            tmp = realloc(tmp, value+1);
			            tmp[value] = 0;
			            ch = byte_getc(src);
			      }
			}
			byte_ungetc(src, ch);

			if (match_fail) {
			      free(tmp);
			      break;
			}

			  /* Matched a decimal value, put it to an argument. */
			item = vpi_scan(argv);
			assert(item);

			val.format = vpiDecStrVal;
			val.value.str = tmp;
			vpi_put_value(item, &val, 0, vpiNoDelay);

			free(tmp);
			rc += 1;
			break;

		      case 'e':
		      case 'f':
		      case 'g':
			item = vpi_scan(argv);
			assert(item);
			rc += scan_format_float(src, item);
			break;

		      case 'h':
		      case 'x':
			match_fail = 1;

			  /* Hex integer */
			tmp = malloc(2);
			value = 0;
			tmp[0] = 0;

			ch = byte_getc(src);
			  /* Skip leading space. */
			while (isspace(ch)) ch = byte_getc(src);

			while (strchr("0123456789abcdefABCDEFxXzZ?_", ch)) {
			      match_fail = 0;
			      if (ch == '?') ch = 'x';
			      if (ch != '_') {
				    ch = tolower(ch);
				    tmp[value++] = ch;
				    tmp = realloc(tmp, value+1);
				    tmp[value] = 0;
			      }
			      ch = byte_getc(src);
			}
			byte_ungetc(src, ch);

			if (match_fail) {
			      free(tmp);
			      break;
			}

			item = vpi_scan(argv);
			assert(item);

			val.format = vpiHexStrVal;
			val.value.str = tmp;
			vpi_put_value(item, &val, 0, vpiNoDelay);
			free(tmp);
			rc += 1;
			break;

		      case 'o':
			match_fail = 1;
			  /* binary integer */
			tmp = malloc(2);
			value = 0;
			tmp[0] = 0;

			ch = byte_getc(src);
			  /* Skip leading space. */
			while (isspace(ch)) ch = byte_getc(src);

			while (strchr("01234567xXzZ?_", ch)) {
			      match_fail = 0;
			      if (ch == '?') ch = 'x';
			      if (ch != '_') {
				    ch = tolower(ch);
				    tmp[value++] = ch;
				    tmp = realloc(tmp, value+1);
				    tmp[value] = 0;
			      }
			      ch = byte_getc(src);
			}
			byte_ungetc(src, ch);

			if (match_fail) {
			      free(tmp);
			      break;
			}

			item = vpi_scan(argv);
			assert(item);

			val.format = vpiOctStrVal;
			val.value.str = tmp;
			vpi_put_value(item, &val, 0, vpiNoDelay);
			free(tmp);
			rc += 1;
			break;

		      case 's':
			item = vpi_scan(argv);
			assert(item);
			rc += scan_format_string(src, item);
			break;

		      case 't':
			item = vpi_scan(argv);
			assert(item);
			rc += scan_format_float_time(callh, src, item);
			break;

		      default:
			vpi_printf("ERROR: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			vpi_printf("$scanf: Unknown format code: %c\n", code);
			break;
		  }
	    }
      }

      free(fmt);

      vpi_free_object(argv);

      val.format = vpiIntVal;
      val.value.integer = rc;
      vpi_put_value(callh, &val, 0, vpiNoDelay);
      return 0;
}

/*
 * Is the object a variable/register or a piece of one.
 */
static int is_assignable_obj(vpiHandle obj)
{
      unsigned rtn = 0;

      assert(obj);

      switch(vpi_get(vpiType, obj)) {
	/* We can not assign to a vpiNetArray. */
	case vpiMemoryWord:
	    if (vpi_get(vpiType, vpi_handle(vpiParent, obj)) == vpiMemory) {
		  rtn = 1;
	    } 
	    break;
	case vpiPartSelect:
	    if (! is_assignable_obj(vpi_handle(vpiParent, obj))) break;
	case vpiIntegerVar:
	case vpiRealVar:
	case vpiReg:
	case vpiTimeVar:
	    rtn = 1;
	    break;
      }

      return rtn;
}

static int sys_check_args(vpiHandle callh, vpiHandle argv, PLI_BYTE8 *name)
{
      vpiHandle arg;
      int cnt = 3, rtn = 0;

	/* The format (2nd) argument must be a string. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires at least three argument.\n", name);
	    return 1;
      }
      if(! is_string_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s format argument must be a string.\n", name);
	    rtn = 1;
      }

	/* The rest of the arguments must be assignable. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires at least three argument.\n", name);
	    return 1;
      }

      do {
	    if (! is_assignable_obj(arg)) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s argument %d (a %s) is not assignable.\n",
		             name, cnt, vpi_get_str(vpiType, arg));
		  rtn = 1;
	    }
	    arg = vpi_scan(argv);
	    cnt += 1;
      } while (arg);

      return rtn;
}

static PLI_INT32 sys_fscanf_compiletf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires at least three argument.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The first argument must be a file descriptor. */
      if (! is_numeric_obj(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument (fd) must be numeric.\n", name);
	    vpi_control(vpiFinish, 1);
	    vpi_free_object(argv);
	    return 0;
      }

      if (sys_check_args(callh, argv, name)) vpi_control(vpiFinish, 1);
      return 0;
}

static PLI_INT32 sys_fscanf_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      s_vpi_value val;
      struct byte_source src;
      FILE *fd;
      errno = 0;

      val.format = vpiIntVal;
      vpi_get_value(vpi_scan(argv), &val);
      fd = vpi_get_file(val.value.integer);
      if (!fd) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n",
	               val.value.integer, name);
	    errno = EBADF;
	    val.format = vpiIntVal;
	    val.value.integer = EOF;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    vpi_free_object(argv);
	    return 0;
      }

      src.str = 0;
      src.fd = fd;
      scan_format(callh, &src, argv);

      return 0;
}

static PLI_INT32 sys_sscanf_compiletf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle reg;

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires at least three argument.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The first argument must be a register or constant string. */
      reg = vpi_scan(argv);  /* This should never be zero. */
      switch(vpi_get(vpiType, reg)) {
	  case vpiReg:
	    break;
	  case vpiConstant:
	  case vpiParameter:
	    if (vpi_get(vpiConstType, reg) == vpiStringConst) break;

	  default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument must be a register or constant "
	               "string.\n", name);
	    vpi_control(vpiFinish, 1);
	    vpi_free_object(argv);
	    return 0;
      }

      if (sys_check_args(callh, argv, name)) vpi_control(vpiFinish, 1);
      return 0;
}

static PLI_INT32 sys_sscanf_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      s_vpi_value val;
      struct byte_source src;
      char *str;

      val.format = vpiStringVal;
      vpi_get_value(vpi_scan(argv), &val);

      str = strdup(val.value.str);
      src.str = str;
      src.fd = 0;
      scan_format(callh, &src, argv);
      free(str);

      return 0;
}

void sys_scanf_register()
{
      s_vpi_systf_data tf_data;

      /*============================== fscanf */
      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname      = "$fscanf";
      tf_data.calltf      = sys_fscanf_calltf;
      tf_data.compiletf   = sys_fscanf_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$fscanf";
      vpi_register_systf(&tf_data);

      /*============================== sscanf */
      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname      = "$sscanf";
      tf_data.calltf      = sys_sscanf_calltf;
      tf_data.compiletf   = sys_sscanf_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$sscanf";
      vpi_register_systf(&tf_data);
}
