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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: sys_display.c,v 1.35 2002/02/06 04:50:04 steve Exp $"
#endif

# include "config.h"

# include  "vpi_user.h"
# include  <assert.h>
# include  <string.h>
# include  <ctype.h>
# include  <stdlib.h>

struct strobe_cb_info {
      char*name;
      int default_format;
      vpiHandle scope;
      vpiHandle*items;
      unsigned nitems;
};

// The number of decimal digits needed to represent a
// nr_bits binary number is floor(nr_bits*log_10(2))+1,
// where log_10(2) = 0.30102999566398....  and I approximate
// this transcendental number as 146/485, to avoid the vagaries
// of floating-point.  The smallest nr_bits for which this
// approximation fails is 2621,
// 2621*log_10(2)=789.9996, but (2621*146+484)/485=790 (exactly).
// In cases like this, all that happens is we allocate one
// unneeded char for the output.  I add a "L" suffix to 146
// to make sure the computation is done as long ints, otherwise
// on a 16-bit int machine (allowed by ISO C) we would mangle
// this computation for bit-length of 224.  I'd like to put
// in a test for nr_bits < LONG_MAX/146, but don't know how
// to fail, other than crashing.
//
// In an April 2000 thread in comp.unix.programmer, with subject
// "integer -> string", I <LRDoolittle@lbl.gov> give the 28/93
// approximation, but overstate its accuracy: that version first
// fails when the number of bits is 289, not 671.
//
// This result does not include space for a trailing '\0', if any.
//
inline static int calc_dec_size(int nr_bits, int is_signed)
{
	int r;
	if (is_signed) --nr_bits;
	r = (nr_bits * 146L + 484) / 485;
	if (is_signed) ++r;
	return r;
}

static int vpi_get_dec_size(vpiHandle item)
{
	return calc_dec_size(
		vpi_get(vpiSize, item),
		vpi_get(vpiSigned, item)==1
	);
}

static void array_from_iterator(struct strobe_cb_info*info, vpiHandle argv)
{
      if (argv) {
	    vpiHandle item;
	    unsigned nitems = 1;
	    vpiHandle*items = malloc(sizeof(vpiHandle));
	    items[0] = vpi_scan(argv);
	    for (item = vpi_scan(argv) ;  item ;  item = vpi_scan(argv)) {
		  items = realloc(items, (nitems+1)*sizeof(vpiHandle));
		  items[nitems] = item;
		  nitems += 1;
	    }

	    info->nitems = nitems;
	    info->items = items;

      } else {
	    info->nitems = 0;
	    info->items = 0;
      }
}


/*
 * If $display discovers a string as a parameter, this function is
 * called to process it as a format string. I need the argv handle as
 * well so that I can look for arguments as I move forward through the
 * string.
 */
static int format_str(vpiHandle scope, unsigned int mcd,
		      char*fmt, int argc, vpiHandle*argv)
{
      s_vpi_value value;
      char buf[256];
      char*cp = fmt;
      char format_char = ' ';
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
		  vpi_mcd_printf(mcd, "%s", buf);
		  cp += cnt;

	    } else if (*cp == '%') {
		  int leading_zero = -1, fsize = -1, ffsize = -1;
		  int do_arg = 0;

		  cp += 1;
		  if (*cp == '0')
		      leading_zero=1;
		  if (isdigit((int)*cp))
			fsize = strtoul(cp, &cp, 10);
		  if (*cp == '.') {
			cp += 1;
			ffsize = strtoul(cp, &cp, 10);
		  }
		  switch (*cp) {
		      case 0:
			break;

		      case 'b':
		      case 'B':
			if (ffsize != -1) {
			     vpi_printf("\nERROR: Illegal format \"%s\"\n", fmt);
			     fsize = -1;
			}
			format_char = 'b';
			do_arg = 1;
			value.format = vpiBinStrVal;
			cp += 1;
			break;

		      case 'd':
		      case 'D':
			if (ffsize != -1) {
			     vpi_printf("\nERROR: Illegal format \"%s\"\n", fmt);
			     fsize = -1;
			}
			format_char = 'd';
			do_arg = 1;
			value.format = vpiDecStrVal;
			cp += 1;
			break;

		      case 'h':
		      case 'H':
		      case 'x':
		      case 'X':
			if (ffsize != -1) {
			     vpi_printf("\nERROR: Illegal format \"%s\"\n", fmt);
			     fsize = -1;
			}
			format_char = 'h';
			do_arg = 1;
			value.format = vpiHexStrVal;
			cp += 1;
			break;

		      case 'c':
		      case 'C':
			if (fsize != -1 && ffsize != -1) {
			     vpi_printf("\nERROR: Illegal format \"%s\"\n", fmt);
			     fsize = -1;
			     ffsize = -1;
			}
			format_char = 'c';
			do_arg = 1;
			value.format = vpiStringVal;
			cp += 1;
			break;

		      case 'm':
		      case 'M':
			if (ffsize != -1) {
			     vpi_printf("\nERROR: Illegal format \"%s\"\n", fmt);
			     fsize = -1;
			}
			if (fsize == -1)
			    fsize = 0;
			assert(scope);
			vpi_mcd_printf(mcd, "%*s",
				       fsize,
				       vpi_get_str(vpiFullName, scope));
			cp += 1;
			break;

		      case 'o':
		      case 'O':
			if (ffsize != -1) {
			     vpi_printf("\nERROR: Illegal format \"%s\"\n", fmt);
			     fsize = -1;
			}
			format_char = 'o';
			do_arg = 1;
			value.format = vpiOctStrVal;
			cp += 1;
			break;

		      case 's':
		      case 'S':
			if (ffsize != -1) {
			     vpi_printf("\nERROR: Illegal format \"%s\"\n", fmt);
			     fsize = -1;
			}
			format_char = 's';
			do_arg = 1;
			value.format = vpiStringVal;
			cp += 1;
			break;

		      case 't':
		      case 'T':
			if (ffsize != -1) {
			     vpi_printf("\nERROR: Illegal format \"%s\"\n", fmt);
			     fsize = -1;
			}
			format_char = 't';
			do_arg = 1;
			value.format = vpiDecStrVal;
			cp += 1;
			break;

		      case '%':
			if (fsize != -1 && ffsize != -1) {
			     vpi_printf("\nERROR: Illegal format \"%s\"\n", fmt);
			     fsize = -1;
			     ffsize = -1;
			}
			vpi_mcd_printf(mcd, "%%");
			cp += 1;
			break;

		      case 'v':
		      case 'V':
		      case 'e':
		      case 'f':
		      case 'g':
			  // new Verilog 2001 format specifiers...
		      case 'l':
		      case 'L':
		      case 'u':
		      case 'U':
		      case 'z':
		      case 'Z':
		        vpi_printf("\nERROR: Unsupported format \"%s\"\n", fmt);
			vpi_mcd_printf(mcd, "%c", *cp);
			cp += 1;
			break;

		      default:
		        vpi_printf("\nERROR: Illegal format \"%s\"\n", fmt);
			vpi_mcd_printf(mcd, "%c", *cp);
			cp += 1;
			break;
		  }

		    /* If we encountered a numeric format string, then
		       grab the number value from the next parameter
		       and display it in the requested format. */
		  if (do_arg) {
			if (idx >= argc) {
			      vpi_printf("\ntoo few arguments for format %s\n",
					 fmt);
			} else {
			      vpi_get_value(argv[idx], &value);
			      if (value.format == vpiSuppressVal){
				  vpi_printf("\nERROR: parameter does not have a printable value!\n", fmt);
				  goto bail_out;
			      }

			      switch(format_char){
			      case 'c':
				  vpi_mcd_printf(mcd, "%c", value.value.str[strlen(value.value.str)-1]);
				  break;

			      case 't':
			      case 'd':
				  if (fsize==-1){
				      // simple %d parameter. 
				      // Size is now determined by the width
				      // of the vector or integer
				      fsize = vpi_get_dec_size(argv[idx]);
				  }

				  vpi_mcd_printf(mcd, "%*s", fsize,
						 value.value.str);
				  break;

			      case 'b':
			      case 'h':
			      case 'x':
			      case 'o':
				  if (fsize==-1){
				      // For hex, oct and binary values, the string is already
				      // prefixed with the correct number of zeros...
				      vpi_mcd_printf(mcd, "%s", value.value.str);
				  }
				  else{ 
				      char* value_str = value.value.str;

				      if (leading_zero==1){
					  // Strip away all leading zeros from string
					  int i=0;
					  while(i< (strlen(value_str)-1) && value_str[i]=='0')
					      i++;
					  
					  value_str += i;
				      }

				      vpi_mcd_printf(mcd, "%*s", fsize, value_str);
				  }
				  break;

			      case 's':
				  if (fsize==-1){
				      vpi_mcd_printf(mcd, "%s", value.value.str);
				  }
				  else{ 
				      char* value_str = value.value.str;

				      if (leading_zero==1){
					  // Remove leading spaces from the value 
					  // string *except* if the argument is a 
					  // constant string... (hey, that's how 
					  // the commerical guys behave...)

					  if (!(vpi_get(vpiType, argv[idx]) == vpiConstant 
					      && vpi_get(vpiConstType, argv[idx]) == vpiStringConst)) {
					      int i=0;
					      // Strip away all leading zeros from string
					      while(i< (strlen(value_str)-1) && value_str[i]==' ')
						  i++;
					  
					      value_str += i;
					  }

				      }

				      vpi_mcd_printf(mcd, "%*s", fsize, value_str);
				  }
				  break;

			      default:
				    if (fsize > 0)
					  vpi_mcd_printf(mcd, "%*s", fsize,
							 value.value.str);
				    else
					  vpi_mcd_printf(mcd, "%s",
							 value.value.str);
			      }

			bail_out:
			      idx++;
			}
		  }

	    } else {

		  cp += 1;
		  switch (*cp) {
		      case 0:
			break;
		      case 'n':
			vpi_mcd_printf(mcd, "\n");
			cp += 1;
			break;
		      case 't':
			vpi_mcd_printf(mcd, "\t");
			cp += 1;
			break;
		      case '\\':
			vpi_mcd_printf(mcd, "\\");
			cp += 1;
			break;
		      case '"':
			vpi_mcd_printf(mcd, "\"");
			cp += 1;
			break;
			
		      default:
			vpi_mcd_printf(mcd, "%c", *cp);
			cp += 1;
		  }
	    }
      }

      return idx;
}

static void do_display(unsigned int mcd, struct strobe_cb_info*info)
{
      s_vpi_value value;
      int idx;
      int size;

      for (idx = 0 ;  idx < info->nitems ;  idx += 1) {
	    vpiHandle item = info->items[idx];

	    switch (vpi_get(vpiType, item)) {

		case 0:
		  vpi_mcd_printf(mcd, " ");
		  break;

		case vpiConstant:
		  if (vpi_get(vpiConstType, item) == vpiStringConst) {
			value.format = vpiStringVal;
			vpi_get_value(item, &value);
			idx += format_str(info->scope, mcd, value.value.str,
					  info->nitems-idx-1,
					  info->items+idx+1);
		  } else {
			value.format = vpiBinStrVal;
			vpi_get_value(item, &value);
			vpi_mcd_printf(mcd, "%s", value.value.str);
		  }
		  break;

		case vpiNet:
		case vpiReg:
		case vpiMemoryWord:
		  value.format = info->default_format;
		  vpi_get_value(item, &value);

		  switch(info->default_format){
		  case vpiDecStrVal:
		      size = vpi_get_dec_size(item);
		      vpi_mcd_printf(mcd, "%*s", size, value.value.str);
		      break;

		  default:
		      vpi_mcd_printf(mcd, "%s", value.value.str);
		  }
		  

		  break;

		case vpiTimeVar:
		  value.format = vpiTimeVal;
		  vpi_get_value(item, &value);
		  vpi_mcd_printf(mcd, "%20u", value.value.time->low);
		  break;

		default:
		  vpi_mcd_printf(mcd, "?");
		  break;
	    }
      }
}

static int get_default_format(char *name)
{
    int default_format;

    switch(name[ strlen(name)-1 ]){
	//  writE/strobE or monitoR or  displaY/fdisplaY
    case 'e':
    case 'r':
    case 'y': default_format = vpiDecStrVal; break;
    case 'h': default_format = vpiHexStrVal; break;
    case 'o': default_format = vpiOctStrVal; break;
    case 'b': default_format = vpiBinStrVal; break;
    default:
	assert(0);
    }

    return default_format;
}

static int sys_display_calltf(char *name)
{
      struct strobe_cb_info info;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle scope = vpi_handle(vpiScope, sys);

      vpiHandle argv = vpi_iterate(vpiArgument, sys);

      assert(scope);
      info.default_format = get_default_format(name);
      info.scope = scope;
      array_from_iterator(&info, argv);

      do_display(5, &info);
      free(info.items);

      if (strncmp(name,"$display",8) == 0)
	    vpi_mcd_printf(5, "\n");

      return 0;
}

/*
 * The strobe implementation takes the parameter handles that are
 * passed to the calltf and puts them in to an array for safe
 * keeping. That array (and other bookkeeping) is passed, via the
 * struct_cb_info object, to the REadOnlySych function strobe_cb,
 * where it is use to perform the actual formatting and printing.
 */

static int strobe_cb(p_cb_data cb)
{
      struct strobe_cb_info*info = (struct strobe_cb_info*)cb->user_data;

      do_display(1, info);

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

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle scope = vpi_handle(vpiScope, sys);

      vpiHandle argv = vpi_iterate(vpiArgument, sys);

      struct strobe_cb_info*info = calloc(1, sizeof(struct strobe_cb_info));

      array_from_iterator(info, argv);
      info->name = strdup(name);
      info->default_format = get_default_format(name);
      info->scope= scope;

      time.type = vpiSimTime;
      time.low = 0;
      time.high = 0;

      cb.reason = cbReadOnlySynch;
      cb.cb_rtn = strobe_cb;
      cb.time = &time;
      cb.obj = 0;
      cb.value = 0;
      cb.user_data = (char*)info;
      vpi_register_cb(&cb);
      return 0;
}

/*
 * The $monitor system task works by managing these static variables,
 * and the cbValueChange callbacks associated with registers and
 * nets. Note that it is proper to keep the state in static variables
 * because there can only be one monitor at a time pending (even
 * though that monitor may be watching many variables).
 */

static struct strobe_cb_info monitor_info = { 0, 0, 0, 0, 0 };
static int monitor_scheduled = 0;
static vpiHandle *monitor_callbacks = 0;

static int monitor_cb_2(p_cb_data cb)
{
      do_display(1, &monitor_info);
      vpi_printf("\n");
      monitor_scheduled = 0;
      return 0;
}

static int monitor_cb_1(p_cb_data cause)
{
      struct t_cb_data cb;
      struct t_vpi_time time;
	/* The user_data of the callback is a pointer to the callback
	   handle. I use this to reschedule the callback if needed. */
      vpiHandle*cbh = (vpiHandle*) (cause->user_data);

      	/* Reschedule this event so that it happens for the next
	   trigger on this variable. */
      time.type = vpiSuppressTime;
      cb.reason = cbValueChange;
      cb.cb_rtn = monitor_cb_1;
      cb.time = &time;
      cb.obj  = cause->obj;
      cb.value = 0;
      cb.user_data = cause->user_data;
      *cbh = vpi_register_cb(&cb);
      
      if (monitor_scheduled) return 0;

	/* This this action caused the first trigger, then schedule
	   the monitor to happen at the end of the time slice and mark
	   it as scheduled. */
      monitor_scheduled += 1;
      time.type = vpiSimTime;
      time.low = 0;
      time.high = 0;

      cb.reason = cbReadOnlySynch;
      cb.cb_rtn = monitor_cb_2;
      cb.time = &time;
      cb.obj = 0;
      cb.value = 0;
      vpi_register_cb(&cb);

      return 0;
}

static int sys_monitor_calltf(char*name)
{
      unsigned idx;
      struct t_cb_data cb;
      struct t_vpi_time time;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle scope = vpi_handle(vpiScope, sys);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);

      if (monitor_callbacks) {
	    for (idx = 0 ;  idx < monitor_info.nitems ;  idx += 1)
		  if (monitor_callbacks[idx])
			vpi_remove_cb(monitor_callbacks[idx]);

	    free(monitor_callbacks);
	    monitor_callbacks = 0;

	    free(monitor_info.items);
	    free(monitor_info.name);
	    monitor_info.items = 0;
	    monitor_info.nitems = 0;
	    monitor_info.name = 0;
      }

      array_from_iterator(&monitor_info, argv);
      monitor_info.name = strdup(name);
      monitor_info.default_format = get_default_format(name);
      monitor_info.scope = scope;

      monitor_callbacks = calloc(monitor_info.nitems, sizeof(vpiHandle));

      time.type = vpiSuppressTime;
      cb.reason = cbValueChange;
      cb.cb_rtn = monitor_cb_1;
      cb.time = &time;
      cb.value = NULL;
      for (idx = 0 ;  idx < monitor_info.nitems ;  idx += 1) {

	    switch (vpi_get(vpiType, monitor_info.items[idx])) {
		case vpiNet:
		case vpiReg:
		    /* Monitoring reg and net values involves setting
		       a collback for value changes. pass the storage
		       pointer for the callback itself as user_data so
		       that the callback can refresh itself. */
		  cb.user_data = (char*)(monitor_callbacks+idx);
		  cb.obj = monitor_info.items[idx];
		  monitor_callbacks[idx] = vpi_register_cb(&cb);
		  break;

	    }
      }

      return 0;
}

/*
 * Implement the $fopen system function.
 */
static int sys_fopen_calltf(char *name)
{
      s_vpi_value val, value, modevalue;
      unsigned char *mode_string;

      vpiHandle call_handle = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, call_handle);
      vpiHandle item = argv ? vpi_scan(argv) : 0;
      vpiHandle mode = item ? vpi_scan(argv) : 0;

      if (item == 0) {
	    vpi_printf("%s: file name parameter missing.\n", name);
	    return 0;
      }

      if (mode == 0) {
	    argv = 0;
      }

      if (vpi_get(vpiType, item) != vpiConstant) {
	    vpi_printf("ERROR: %s parameter must be a constant\n", name);
	    vpi_free_object(argv);
	    return 0;
      }

      if (vpi_get(vpiConstType, item) != vpiStringConst) {
	    vpi_printf("ERROR: %s parameter must be a string.\n", name);
	    vpi_free_object(argv);
	    return 0;
      }

      if (mode == 0) {
            mode_string = "w";
      } else {
	    if (vpi_get(vpiType, mode) != vpiConstant) {
		vpi_printf("ERROR: %s parameter must be a constant\n", name);
		vpi_free_object(argv);
	        return 0;
	    }

           if (vpi_get(vpiConstType, mode) != vpiStringConst) {
               vpi_printf("ERROR: %s parameter must be a string.\n", name);
               vpi_free_object(argv);
               return 0;
           }
           modevalue.format = vpiStringVal;
           vpi_get_value(mode, &modevalue);
           mode_string = modevalue.value.str;
      }

      value.format = vpiStringVal;
      vpi_get_value(item, &value);

      val.format = vpiIntVal;
      val.value.integer = vpi_mcd_open_x( value.value.str, mode_string );

      vpi_put_value(call_handle, &val, 0, vpiNoDelay);

      return 0;
}

static int sys_fopen_sizetf(char*x)
{
      return 32;
}

/* Implement $fdisplay and $fwrite.  
 * Perhaps this could be merged into sys_display_calltf.
 */
static int sys_fdisplay_calltf(char *name)
{
      struct strobe_cb_info info;
      unsigned int mcd;
      int type;
      s_vpi_value value;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle scope = vpi_handle(vpiScope, sys);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);

      if (item == 0) {
	    vpi_printf("%s: mcd parameter missing.\n", name);
	    return 0;
      }

      type = vpi_get(vpiType, item);
      if (type != vpiReg && type != vpiRealVal) {
	    vpi_printf("ERROR: %s mcd parameter must be of integral, got vpiType=%d\n",
		       name, type);
	    vpi_free_object(argv);
	    return 0;
      }

      value.format = vpiIntVal;
      vpi_get_value(item, &value);
      mcd = value.value.integer;

      assert(scope);
      info.default_format = get_default_format(name);
      info.scope = scope;
      array_from_iterator(&info, argv);
      do_display(mcd, &info);
      free(info.items);

      if (strncmp(name,"$fdisplay",9) == 0)
	    vpi_mcd_printf(mcd, "\n");

      return 0;
}


/*
 * Implement $fclose system function
 */
static int sys_fclose_calltf(char *name)
{
      unsigned int mcd;
      int type;
      s_vpi_value value;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);

      if (item == 0) {
	    vpi_printf("%s: mcd parameter missing.\n", name);
	    return 0;
      }
      type = vpi_get(vpiType, item);
      if (type != vpiReg && type != vpiRealVal) {
	    vpi_printf("ERROR: %s mcd parameter must be of integral type, got vpiType=%d\n",
		       name, type);
	    vpi_free_object(argv);
	    return 0;
      }

      value.format = vpiIntVal;
      vpi_get_value(item, &value);
      mcd = value.value.integer;

      vpi_mcd_close(mcd);
      return 0;
}

static int sys_fputc_calltf(char *name)
{
      unsigned int mcd;
      int type;
      unsigned char x;
      s_vpi_value value, xvalue;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);

      if (item == 0) {
	    vpi_printf("%s: mcd parameter missing.\n", name);
	    return 0;
      }

      type = vpi_get(vpiType, item);
      if (type != vpiReg && type != vpiRealVal) {
	    vpi_printf("ERROR: %s mcd parameter must be of integral, got vpiType=%d\n",
		       name, type);
	    vpi_free_object(argv);
	    return 0;
      }

      value.format = vpiIntVal;
      vpi_get_value(item, &value);
      mcd = value.value.integer;

      item = vpi_scan(argv);

      xvalue.format = vpiIntVal;
      vpi_get_value(item, &xvalue);
      x = xvalue.value.integer;

      return vpi_mcd_fputc( mcd, x );
}

static int sys_fgetc_calltf(char *name)
{
      unsigned int mcd;
      int type;
      s_vpi_value value, rval;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);

      if (item == 0) {
	    vpi_printf("%s: mcd parameter missing.\n", name);
	    return 0;
      }

      type = vpi_get(vpiType, item);
      if (type != vpiReg && type != vpiRealVal) {
	    vpi_printf("ERROR: %s mcd parameter must be of integral, got vpiType=%d\n",
		       name, type);
	    vpi_free_object(argv);
	    return 0;
      }

      value.format = vpiIntVal;
      vpi_get_value(item, &value);
      mcd = value.value.integer;

      rval.format = vpiIntVal;
      rval.value.integer = vpi_mcd_fgetc( mcd );

      vpi_put_value(sys, &rval, 0, vpiNoDelay);

      return 0;
}

static int sys_fgetc_sizetf(char*x)
{
      return 32;
}

void sys_display_register()
{
      s_vpi_systf_data tf_data;

      //============================== display
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$display";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$display";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$displayh";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$displayh";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$displayo";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$displayo";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$displayb";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$displayb";
      vpi_register_systf(&tf_data);

      //============================== write
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$write";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$write";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$writeh";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$writeh";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$writeo";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$writeo";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$writeb";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$writeb";
      vpi_register_systf(&tf_data);

      //============================== strobe
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$strobe";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$strobe";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$strobeh";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$strobeh";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$strobeo";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$strobeo";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$strobeb";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$strobeb";
      vpi_register_systf(&tf_data);

      //============================== monitor
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitor";
      tf_data.calltf    = sys_monitor_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitor";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitorh";
      tf_data.calltf    = sys_monitor_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitorh";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitoro";
      tf_data.calltf    = sys_monitor_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitoro";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitorb";
      tf_data.calltf    = sys_monitor_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitorb";
      vpi_register_systf(&tf_data);

      //============================== fopen
      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$fopen";
      tf_data.calltf    = sys_fopen_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = sys_fopen_sizetf;
      tf_data.user_data = "$fopen";
      vpi_register_systf(&tf_data);

      //============================== fclose
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fclose";
      tf_data.calltf    = sys_fclose_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fclose";
      vpi_register_systf(&tf_data);

      //============================== fdisplay
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fdisplay";
      tf_data.calltf    = sys_fdisplay_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fdisplay";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fdisplayh";
      tf_data.calltf    = sys_fdisplay_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fdisplayh";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fdisplayo";
      tf_data.calltf    = sys_fdisplay_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fdisplayo";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fdisplayb";
      tf_data.calltf    = sys_fdisplay_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fdisplayb";
      vpi_register_systf(&tf_data);

      //============================== fwrite
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fwrite";
      tf_data.calltf    = sys_fdisplay_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fwrite";
      vpi_register_systf(&tf_data);

      //============================== fputc
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fputc";
      tf_data.calltf    = sys_fputc_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fputc";
      vpi_register_systf(&tf_data);

      //============================== fgetc
      tf_data.type      = vpiSysFunc;
      tf_data.tfname    = "$fgetc";
      tf_data.calltf    = sys_fgetc_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = sys_fgetc_sizetf;
      tf_data.user_data = "$fgetc";
      vpi_register_systf(&tf_data);
}


/*
 * $Log: sys_display.c,v $
 * Revision 1.35  2002/02/06 04:50:04  steve
 *  Detect and skip suppressed values in display
 *
 * Revision 1.34  2002/01/22 00:18:10  steve
 *  Better calcuation of dec string width (Larry Doolittle)
 *
 * Revision 1.33  2002/01/15 03:23:34  steve
 *  Default widths pad out as per the standard,
 *  add $displayb/o/h et al., and some better
 *  error messages for incorrect formats.
 *
 * Revision 1.32  2002/01/11 04:48:01  steve
 *  Add the %c format, and some warning messages.
 *
 * Revision 1.31  2001/11/02 05:56:47  steve
 *  initialize scope for %m in $fdisplay.
 *
 * Revision 1.30  2001/10/25 04:19:53  steve
 *  VPI support for callback to return values.
 *
 * Revision 1.29  2001/08/16 03:26:04  steve
 *  Add some missing print escape sequences.
 *
 * Revision 1.28  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.27  2001/07/16 18:40:19  steve
 *  Add a stdlog output for vvp, and vvp options
 *  to direct them around. (Stephan Boettcher.)
 *
 * Revision 1.26  2001/07/11 02:22:17  steve
 *  Manually create the stage-2 callback structure.
 *
 * Revision 1.25  2001/06/25 03:11:41  steve
 *  More robust about incorrect arguments.
 *
 * Revision 1.24  2001/03/22 02:23:40  steve
 *  fgetc patch from Peter Monta.
 *
 * Revision 1.23  2001/03/18 00:31:32  steve
 *  $display can take 0 arguments.
 *
 * Revision 1.22  2001/02/10 19:50:33  steve
 *  Slightly more specific error message.
 *
 * Revision 1.21  2000/12/02 02:40:56  steve
 *  Support for %s in $display (PR#62)
 *
 * Revision 1.20  2000/11/04 05:49:22  steve
 *  Integrate parameter count changes (PR#34)
 *
 * Revision 1.19  2000/11/04 01:52:57  steve
 *  Scope information is needed by all types of display tasks.
 *
 * Revision 1.18  2000/10/28 00:51:42  steve
 *  Add scope to threads in vvm, pass that scope
 *  to vpi sysTaskFunc objects, and add vpi calls
 *  to access that information.
 *
 *  $display displays scope in %m (PR#1)
 *
 * Revision 1.17  2000/08/20 17:49:05  steve
 *  Clean up warnings and portability issues.
 *
 * Revision 1.16  2000/05/31 02:15:43  steve
 *  typo: fix vpiReadVal to vpiRealVal
 *
 * Revision 1.15  2000/05/09 00:02:29  steve
 *  Remove test print.
 *
 * Revision 1.14  2000/05/07 18:20:07  steve
 *  Import MCD support from Stephen Tell, and add
 *  system function parameter support to the IVL core.
 *
 * Revision 1.13  2000/04/21 02:00:35  steve
 *  exit if hex value is missing.
 *
 * Revision 1.12  2000/03/31 07:08:39  steve
 *  allow cancelling of cbValueChange events.
 *
 * Revision 1.11  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.10  2000/02/13 19:18:27  steve
 *  Accept memory words as parameter to $display.
 *
 * Revision 1.9  1999/11/07 02:25:07  steve
 *  Add the $monitor implementation.
 *
 * Revision 1.8  1999/11/06 23:32:14  steve
 *  Unify display and strobe format routines.
 *
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
 */

