/*
 * Copyright (c) 1999-2010 Stephen Williams (steve@icarus.com)
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

# include "vpi_config.h"

# include  "vpi_user.h"
# include  <assert.h>
# include  <string.h>
# include  <ctype.h>
# include  <stdio.h>
# include  <stdlib.h>
# include  <math.h>

#define IS_MCD(mcd)     !((mcd)>>31&1)

/* Printf wrapper to handle both MCD/FD */
static PLI_INT32 my_mcd_printf(PLI_UINT32 mcd, const char *fmt, ...)
{
      int r = 0;

      va_list ap;
      va_start(ap, fmt);

      if (IS_MCD(mcd)) {
	    r = vpi_mcd_vprintf(mcd, fmt, ap);
      } else {
	    FILE *fp = vpi_get_file(mcd);
	    if (fp) r = vfprintf(fp, fmt, ap);
      }

      va_end(ap);
      return r;
}

struct timeformat_info_s {
      int units;
      unsigned prec;
      char*suff;
      unsigned width;
};

struct timeformat_info_s timeformat_info = { 0, 0, 0, 20 };

struct strobe_cb_info {
      char*name;
      int default_format;
      vpiHandle scope;
      vpiHandle*items;
      unsigned nitems;
      unsigned mcd;
};

int is_constant(vpiHandle obj)
{
      if (vpi_get(vpiType, obj) == vpiConstant)
	    return vpiConstant;
      if (vpi_get(vpiType, obj) == vpiParameter)
	    return vpiParameter;

      return 0;
}

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
	    if (items[0] == 0) {
		  free(items);
		  info->nitems = 0;
		  info->items  = 0;
		  return;
	    }

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
 * This function writes the time value into the mcd target with the
 * proper format. The mcd is the destination file.
 *
 * The fsize is the width of the field to use. Normally, this is -1 to
 * reflect the format string "%t". It may also be 0 for the format
 * string "%0t". This formatter also allows for the nonstandard use of
 * positive values to enforce a with to override the width given in
 * the $timeformat system task.
 *
 * The value argument is the time value as a decimal string. There are
 * no leading zeros in this string, and the units of the value are
 * given in the units argument.
 */
static void format_time(unsigned mcd, int fsize,
			const char*value, int time_units)
{
      char buf[256];
      const char*cp;
      char*bp, *start_address;

      int idx;
      unsigned int fusize;
      int fraction_chars, fraction_pad, value_chop, whole_fill;

	/* This is the format precision expressed as the power of 10
	   of the desired precision. The following code uses this
	   format to be consistent with the units specifications. */
      int format_precision = timeformat_info.units - timeformat_info.prec;

	/* If the fsize is <0, then use the value from the
	   $timeformat. If the fsize is >=0, then it overrides the
	   $timeformat value. */
      fusize = (fsize<0) ? timeformat_info.width : (unsigned) fsize;

      assert(fusize < (sizeof buf - 1));


	/* This is the number of characters to the right of the
	   decimal point. This is defined completely by the
	   timeformat. It is legal for the precision to be larger than
	   the units, and in this case there will be no fraction_chars
	   at all. */
      fraction_chars = timeformat_info.units - format_precision;
      if (fraction_chars < 0)
	    fraction_chars = 0;

	/* This is the number of zeros I must add to the value to get
	   the desired precision within the fraction. If this value is
	   greater than 0, the value does not have enough characters,
	   so I will be adding zeros. */

      fraction_pad = time_units - format_precision;
      if (fraction_pad < 0)
	    fraction_pad = 0;
      if (fraction_pad > fraction_chars)
	    fraction_pad = fraction_chars;


	/* This is the number of characters of excess precision in the
	   supplied value. This many characters are chopped from the
	   least significant end. */
      value_chop = format_precision - time_units;
      if (value_chop < 0)
	    value_chop = 0;

	/* This is the number of zeros that I must add to the integer
	   part of the output string to pad the value out to the
	   desired units. This will only have a non-zero value if the
	   units of the value is greater than the desired units.

	   Detect the special case where the value is 0. In this case,
	   do not do any integer filling ever. The output should be
	   padded with blanks in that case. */
      whole_fill = time_units - timeformat_info.units;
      if (whole_fill < 0)
	    whole_fill = 0;
      if (strcmp(value,"0") == 0)
	    whole_fill = 0;

	/* This is the expected start address of the output. It
	   accounts for the f(u)size value that is chosen. The output
	   will be filled out to complete the buffer. */
      if (fusize == 0)
	    start_address = buf;
      else
	    start_address = buf + sizeof buf - fusize - 1;

	/* Now start the character pointers, ready to start copying
	   the value into the format. */
      cp = value + strlen(value);
      if (value_chop > (cp - value))
	    cp = value;
      else
	    cp -= value_chop;

      bp = buf + sizeof buf;
      *--bp = 0;


	/* Write the suffix. */
      bp -= strlen(timeformat_info.suff);
      strcpy(bp, timeformat_info.suff);

	/* Write the padding needed to fill out the fraction. */
      for (idx = 0 ;  idx < fraction_pad ;  idx += 1)
	    *--bp = '0';

	/* Subtract the pad from the needed chars. */
      assert(fraction_pad <= fraction_chars);
      fraction_chars -= fraction_pad;
      fraction_pad = 0;

	/* Write the fraction chars. */
      for (idx = 0 ;  idx < fraction_chars ;  idx += 1) {
	    if (cp > value)
		  *--bp = *--cp;
	    else
		  *--bp = '0';

	    assert(cp >= value);
      }

	/* Write the decimal point, if needed. */
      if (timeformat_info.prec > 0)
	    *--bp = '.';

	/* Fill the gap between the value and the decimal point. */
      for (idx = 0 ;  idx < whole_fill ;  idx += 1)
	    *--bp = '0';

	/* Write the integer part of the value. */
      while (cp > value) {
	    *--bp = *--cp;
      }

	/* Fill the leading characters to make up the desired
	   width. This may require a '0' if the last character
	   written was the decimal point. */
      if (fusize > 0) {
	    while (bp > start_address) {
		  if (*bp == '.')
			*--bp = '0';
		  else
			*--bp = ' ';
	    }
      } else {
	    if (*bp == '.')
		  *--bp = '0';
      }


      my_mcd_printf(mcd, "%s", bp);
}

static void format_time_real(unsigned mcd, int fsize,
			     double value, int time_units)
{

	/* The time_units is the units of the current scope, and also
	   of the value. If the scope units are different from the
	   format specifier units, then scale the value. */
      if (time_units != timeformat_info.units)
	    value *= pow(10.0, time_units - timeformat_info.units);

	/* The timeformat_info.prec is the number of digits after the
	   decimal point, no matter what the units. */
      my_mcd_printf(mcd, "%0.*f%s", timeformat_info.prec, value,
		     timeformat_info.suff);
}


static void format_strength(unsigned int mcd, s_vpi_value*value)
{
      char str[4];
      vpip_format_strength(str, value);
      my_mcd_printf(mcd, "%s", str);
}

static void format_error_msg(const char*msg, int leading_zero,
			     int fsize, int ffsize, char fmt)
{
      if ((fsize < 0) && (ffsize < 0)) {
	    if (leading_zero > 0)
		  vpi_printf("\nERROR: %s: %%0%c\n", msg, fmt);
	    else
		  vpi_printf("\nERROR: %s: %%%c\n", msg, fmt);

      } else if (ffsize < 0) {
	    if ((leading_zero > 0) && (fsize > 0))
		  vpi_printf("\nERROR: %s: %%0%d%c\n", msg,
			     fsize, fmt);
	    else
		  vpi_printf("\nERROR: %s: %%%d%c\n", msg,
			     fsize, fmt);

      } else {
	    vpi_printf("\nERROR: %s: %%%d.%d%c\n", msg,
		       fsize, ffsize, fmt);
      }
}

/*
 * The format_str uses this function to do the special job of
 * interpreting the next item depending on the format code. The caller
 * has already parsed the %x.yf format string.
 *
 * The return code is the number of arguments that were consumed.
 */
static int format_str_char(vpiHandle scope, unsigned int mcd,
			   int leading_zero, int fsize, int ffsize,
			   char fmt, int argc, vpiHandle*argv, int idx)
{
      s_vpi_value value;
      int use_count = 0;

	/* Time units of the current scope. */
      int time_units = vpi_get(vpiTimeUnit, scope);

      switch (fmt) {

	  case 0:
	    return 0;

	  case '%':
	    if (fsize != -1 && ffsize != -1) {
		  format_error_msg("Illegal format", leading_zero,
				       fsize, ffsize, fmt);
		  fsize = -1;
		  ffsize = -1;
	    }

	    my_mcd_printf(mcd, "%%");

	    use_count = 0;
	    break;

	      // new Verilog 2001 format specifiers...
	  case 'l':
	  case 'L':
	  case 'u':
	  case 'U':
	  case 'z':
	  case 'Z':
	    format_error_msg("Unsupported format", leading_zero,
				 fsize, ffsize, fmt);
	    my_mcd_printf(mcd, "%c", fmt);

	    use_count = 0;
	    break;

	  default:
	    format_error_msg("Illegal format", leading_zero,
			     fsize, ffsize, fmt);
	    my_mcd_printf(mcd, "%c", fmt);
	    break;

	      /* Print numeric value in binary/hex/octal format. */
	  case 'b':
	  case 'B':
	  case 'h':
	  case 'H':
	  case 'o':
	  case 'O':
	  case 'x':
	  case 'X':
	    if (ffsize != -1) {
		  format_error_msg("Illegal format", leading_zero,
				       fsize, ffsize, fmt);
		  fsize = -1;
	    }

	    if (idx >= argc) {
		  format_error_msg("Missing Argument", leading_zero,
				   fsize, ffsize, fmt);
		  return 0;
	    }

	    switch (fmt) {
		case 'b':
		case 'B':
		  value.format = vpiBinStrVal;
		  break;
		case 'h':
		case 'H':
		case 'x':
		case 'X':
		  value.format = vpiHexStrVal;
		  break;
		case 'o':
		case 'O':
		  value.format = vpiOctStrVal;
		  break;
	    }

	    vpi_get_value(argv[idx], &value);
	    if (value.format == vpiSuppressVal){
		  format_error_msg("Incompatible value", leading_zero,
				   fsize, ffsize, fmt);
		  return 1;
	    }

	    { char* value_str = value.value.str;
	      if (leading_zero==1){
		      // Strip away all leading zeros from string
		    unsigned int i=0;
		    while(i< (strlen(value_str)-1) && value_str[i]=='0')
			  i++;
		    value_str += i;
	      }

	      my_mcd_printf(mcd, "%*s", fsize, value_str);
	    }

	    use_count = 1;
	    break;

	      /* Print character */
	  case 'c':
	  case 'C':
	    if (fsize != -1 && ffsize != -1) {
		  format_error_msg("Illegal format", leading_zero,
				       fsize, ffsize, fmt);
		  fsize = -1;
		  ffsize = -1;
	    }

	    if (idx >= argc) {
		  format_error_msg("Missing Argument", leading_zero,
				   fsize, ffsize, fmt);
		  return 0;
	    }

	    value.format = vpiStringVal;
	    vpi_get_value(argv[idx], &value);
	    if (value.format == vpiSuppressVal){
		  format_error_msg("Incompatible value", leading_zero,
				   fsize, ffsize, fmt);
		  return 1;
	    }

	    my_mcd_printf(mcd, "%c", value.value.str[strlen(value.value.str)-1]);

	    use_count = 1;
	    break;

	      /* Print numeric value is decimal integer format. */
	  case 'd':
	  case 'D':
	    if (ffsize != -1) {
		  format_error_msg("Illegal format", leading_zero,
				       fsize, ffsize, fmt);
		  fsize = -1;
	    }

	    if (idx >= argc) {
		  format_error_msg("Missing Argument", leading_zero,
				   fsize, ffsize, fmt);
		  return 0;
	    }

	    value.format = vpiDecStrVal;
	    vpi_get_value(argv[idx], &value);
	    if (value.format == vpiSuppressVal){
		  format_error_msg("Incompatible value", leading_zero,
				   fsize, ffsize, fmt);
		  return 1;
	    }

	    if (fsize==-1){
		    // simple %d parameter, or %0d parameter.
		    // Size is now determined by the width
		    // of the vector or integer. If %0d, then
		    // set the size to 0 so that the minimum
		    // size is used.
		  fsize = (leading_zero==1)
			? 0
			: vpi_get_dec_size(argv[idx]);
	    }

	    my_mcd_printf(mcd, "%*s", fsize, value.value.str);

	    use_count = 1;
	    break;

	  case 'e':
	  case 'E':
	    if (idx >= argc) {
		  format_error_msg("Missing Argument", leading_zero,
				   fsize, ffsize, fmt);
		  return 0;
	    }

	    value.format = vpiRealVal;
	    vpi_get_value(argv[idx], &value);
	    if (value.format == vpiSuppressVal){
		  format_error_msg("Incompatible value", leading_zero,
				   fsize, ffsize, fmt);
		  return 1;
	    }

	    my_mcd_printf(mcd, "%*.*g", fsize, ffsize, value.value.real);

	    use_count = 1;
	    break;

	  case 'f':
	  case 'F':
	    if (idx >= argc) {
		  format_error_msg("Missing Argument", leading_zero,
				   fsize, ffsize, fmt);
		  return 0;
	    }

	    value.format = vpiRealVal;
	    vpi_get_value(argv[idx], &value);
	    if (value.format == vpiSuppressVal){
		  format_error_msg("Incompatible value", leading_zero,
				   fsize, ffsize, fmt);
		  return 1;
	    }

	    my_mcd_printf(mcd, "%*.*f", fsize, ffsize, value.value.real);

	    use_count = 1;
	    break;

	  case 'g':
	  case 'G':
	    if (idx >= argc) {
		  format_error_msg("Missing Argument", leading_zero,
				   fsize, ffsize, fmt);
		  return 0;
	    }

	    value.format = vpiRealVal;
	    vpi_get_value(argv[idx], &value);
	    if (value.format == vpiSuppressVal){
		  format_error_msg("Incompatible value", leading_zero,
				   fsize, ffsize, fmt);
		  return 1;
	    }

	    my_mcd_printf(mcd, "%*.*g", fsize, ffsize, value.value.real);

	    use_count = 1;
	    break;

	      /* Print the current scope. */
	  case 'm':
	  case 'M':
	    if (ffsize != -1) {
		  format_error_msg("Illegal format", leading_zero,
				       fsize, ffsize, fmt);
		  fsize = -1;
	    }
	    if (fsize == -1)
		  fsize = 0;
	    assert(scope);
	    my_mcd_printf(mcd, "%*s",
			   fsize,
			   vpi_get_str(vpiFullName, scope));
	    break;


	      /* Print vector as a string value. */
	  case 's':
	  case 'S':
	    if (ffsize != -1) {
		  format_error_msg("Illegal format", leading_zero,
				       fsize, ffsize, fmt);
		  fsize = -1;
	    }

	    value.format = vpiStringVal;
	    vpi_get_value(argv[idx], &value);
	    if (value.format == vpiSuppressVal){
		  format_error_msg("Incompatible value", leading_zero,
				   fsize, ffsize, fmt);
		  return 1;
	    }

	    if (fsize==-1){
		  fsize = (vpi_get(vpiSize, argv[idx]) + 7) / 8;
		  my_mcd_printf(mcd, "%*s", fsize, value.value.str);

	    } else {
		  char* value_str = value.value.str;
		  my_mcd_printf(mcd, "%*s", fsize, value_str);
	    }

	    use_count = 1;
	    break;

	  case 't':
	  case 'T':
	    if (ffsize != -1) {
		  format_error_msg("Illegal format", leading_zero,
				       fsize, ffsize, fmt);
		  fsize = -1;
	    }

	    if (idx >= argc) {
		  format_error_msg("Missing Argument", leading_zero,
				   fsize, ffsize, fmt);
		  return 0;
	    }

	    if (is_constant(argv[idx])
		&& (vpi_get(vpiConstType, argv[idx]) == vpiRealConst)) {

		  value.format = vpiRealVal;
		  vpi_get_value(argv[idx], &value);
		  format_time_real(mcd, fsize, value.value.real, time_units);

	    } else {

		  value.format = vpiDecStrVal;
		  vpi_get_value(argv[idx], &value);
		  if (value.format == vpiSuppressVal){
			format_error_msg("Incompatible value", leading_zero,
					 fsize, ffsize, fmt);
			return 1;
		  }

		  format_time(mcd, fsize, value.value.str, time_units);
	    }

	    use_count = 1;
	    break;

	  case 'v':
	  case 'V':
	    value.format = vpiStrengthVal;
	    vpi_get_value(argv[idx], &value);
	    if (value.format == vpiSuppressVal){
		  format_error_msg("Incompatible value", leading_zero,
				   fsize, ffsize, fmt);
		  return 1;
	    }

	    format_strength(mcd, &value);

	    use_count = 1;
	    break;

      }

      return use_count;
}

/*
 * If $display discovers a string as a parameter, this function is
 * called to process it as a format string. I need the argv handle as
 * well so that I can look for arguments as I move forward through the
 * string.
 */
static int format_str(vpiHandle scope, unsigned int mcd,
		      char*format, int argc, vpiHandle*argv)
{
      char buf[256], fmt[256];
      char*cp = fmt;
      int idx;

      assert(format);

      /*
       * Copy format out of value buffer since it will be
       * clobbered by successive vpi_get_value() calls.
       */
      strncpy(fmt, format, sizeof fmt - 1);
      fmt[sizeof fmt - 1] = 0;

      idx = 0;

      while (*cp) {
	    size_t cnt = strcspn(cp, "%\\");
	    if (cnt > 0) {
		  if (cnt >= sizeof buf)
			cnt = sizeof buf - 1;
		  strncpy(buf, cp, cnt);
		  buf[cnt] = 0;
		  my_mcd_printf(mcd, "%s", buf);
		  cp += cnt;

	    } else if (*cp == '%') {
		  int leading_zero = -1, ljust = 1, fsize = -1, ffsize = -1;

		  cp += 1;
		  if (*cp == '-') {
		      ljust=-1;
		      cp += 1;
		  }
		  if (*cp == '0')
		      leading_zero=1;
		  if (isdigit((int)*cp))
			fsize = ljust * strtoul(cp, &cp, 10);
		  if (*cp == '.') {
			cp += 1;
			ffsize = strtoul(cp, &cp, 10);
		  }

		  idx += format_str_char(scope, mcd, leading_zero,
					 fsize, ffsize, *cp,
					 argc, argv, idx);
		  cp += 1;

	    } else {

		  cp += 1;
		  switch (*cp) {
		      case 0:
			break;
		      case 'n':
			my_mcd_printf(mcd, "\n");
			cp += 1;
			break;
		      case 't':
			my_mcd_printf(mcd, "\t");
			cp += 1;
			break;
		      case '\\':
			my_mcd_printf(mcd, "\\");
			cp += 1;
			break;
		      case '"':
			my_mcd_printf(mcd, "\"");
			cp += 1;
			break;

		      case '0':
		      case '1':
		      case '2':
		      case '3':
		      case '4':
		      case '5':
		      case '6':
		      case '7':
			if (isdigit(cp[0])
			    && isdigit(cp[1])
			    && isdigit(cp[2])) {
			        /* handle octal escapes (e.g. "\015" is CR)*/
			      my_mcd_printf(mcd, "%c",
					     (cp[2] - '0') +
					     8 * ((cp[1] - '0') +
						  8 * (cp[0] - '0')));
			      cp += 3;
			} else {
			      my_mcd_printf(mcd, "%c", *cp);
			      cp += 1;
			}
			break;

		      default:
			my_mcd_printf(mcd, "%c", *cp);
			cp += 1;
		  }
	    }
      }

      return idx;
}

static void do_display_numeric(unsigned int mcd,
			       struct strobe_cb_info*info,
			       vpiHandle item)
{
      int size;
      s_vpi_value value;

      value.format = info->default_format;
      vpi_get_value(item, &value);

      switch(info->default_format){
	  case vpiDecStrVal:
	    size = vpi_get_dec_size(item);
	    my_mcd_printf(mcd, "%*s", size, value.value.str);
	    break;

	  default:
	    my_mcd_printf(mcd, "%s", value.value.str);
      }
}

static void do_display(unsigned int mcd, struct strobe_cb_info*info)
{
      char*fmt;
      s_vpi_value value;
      unsigned int idx;

      for (idx = 0 ;  idx < info->nitems ;  idx += 1) {
	    vpiHandle item = info->items[idx];

	    switch (vpi_get(vpiType, item)) {

		case 0:
		  my_mcd_printf(mcd, " ");
		  break;

		case vpiConstant:
		case vpiParameter:
		  if (vpi_get(vpiConstType, item) == vpiStringConst) {
			value.format = vpiStringVal;
			vpi_get_value(item, &value);
			fmt = strdup(value.value.str);
			idx += format_str(info->scope, mcd, fmt,
					  info->nitems-idx-1,
					  info->items+idx+1);
			free(fmt);
		  } else {
			do_display_numeric(mcd, info, item);
		  }
		  break;

		case vpiNet:
		case vpiReg:
		case vpiIntegerVar:
		case vpiMemoryWord:
		  do_display_numeric(mcd, info, item);
		  break;

		case vpiTimeVar:
		  value.format = vpiTimeVal;
		  vpi_get_value(item, &value);
		  my_mcd_printf(mcd, "%20u", value.value.time->low);
		  break;

		case vpiRealVar:
		  value.format = vpiRealVal;
		  vpi_get_value(item, &value);
		  my_mcd_printf(mcd, "%f", value.value.real);
		  break;

		case vpiSysFuncCall: {
		      char*tmp = vpi_get_str(vpiName, item);
		      vpiHandle scope = vpi_handle(vpiScope, item);

		      if (strcmp(tmp,"$time") == 0) {
			    value.format = vpiTimeVal;
			    vpi_get_value(item, &value);
			    my_mcd_printf(mcd, "%20u", value.value.time->low);
		      } else if (strcmp(tmp,"$realtime") == 0) {
			    int time_units = vpi_get(vpiTimeUnit, scope);
			    int time_prec = vpi_get(vpiTimePrecision, 0);
			    int use_prec = time_units - time_prec;
			    if (use_prec < 0)
				  use_prec = 0;

			    value.format = vpiRealVal;
			    vpi_get_value(item, &value);
			    my_mcd_printf(mcd, "%0.*f", use_prec,
					   value.value.real);
		      } else {
			    my_mcd_printf(mcd, "<%s>", tmp);
		      }
		      break;
		}

		default:
		  my_mcd_printf(mcd, "?");
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

static PLI_INT32 sys_display_calltf(char *name)
{
      struct strobe_cb_info*info;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);

      info = vpi_get_userdata(sys);
      if (info == 0) {
	    vpiHandle scope = vpi_handle(vpiScope, sys);
	    vpiHandle argv = vpi_iterate(vpiArgument, sys);

	    assert(scope);
	    info = malloc(sizeof (struct strobe_cb_info));
	    info->default_format = get_default_format(name);
	    info->scope = scope;
	    array_from_iterator(info, argv);

	    vpi_put_userdata(sys, info);
      }

      do_display(1, info);

      if (strncmp(name,"$display",8) == 0)
	    my_mcd_printf(1, "\n");

      return 0;
}

/*
 * The strobe implementation takes the parameter handles that are
 * passed to the calltf and puts them in to an array for safe
 * keeping. That array (and other bookkeeping) is passed, via the
 * struct_cb_info object, to the REadOnlySych function strobe_cb,
 * where it is use to perform the actual formatting and printing.
 */

static PLI_INT32 strobe_cb(p_cb_data cb)
{
      struct strobe_cb_info*info = (struct strobe_cb_info*)cb->user_data;

      do_display(info->mcd, info);
      my_mcd_printf(info->mcd, "\n");

      free(info->name);
      free(info->items);
      free(info);

      return 0;
}

static PLI_INT32 sys_strobe_calltf(char*name)
{
      struct t_cb_data cb;
      struct t_vpi_time time;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle scope = vpi_handle(vpiScope, sys);

      vpiHandle argv = vpi_iterate(vpiArgument, sys);

      struct strobe_cb_info*info = calloc(1, sizeof(struct strobe_cb_info));
      if(name[1] == 'f') {
	      vpiHandle item = vpi_scan(argv);
	      int type;
	      s_vpi_value value;

	      if (item == 0) {
		      vpi_printf("%s: mcd parameter missing.\n", name);
		      return 0;
	      }

	      type = vpi_get(vpiType, item);
	      switch (type) {
	      case vpiReg:
	      case vpiRealVar:
	      case vpiIntegerVar:
	      case vpiNet:
		      break;

	      case vpiConstant:
		      switch (vpi_get(vpiConstType, item)) {
		      case vpiDecConst:
		      case vpiBinaryConst:
		      case vpiOctConst:
		      case vpiHexConst:
			      break;
		      default:
			      vpi_printf("ERROR: %s mcd parameter must be integral", name);
			      vpi_printf(", got vpiType=vpiConstant, vpiConstType=%d\n",
					 vpi_get(vpiConstType, item));
			      vpi_free_object(argv);
			      return 0;
		      }
		      break;

	      default:
		      vpi_printf("ERROR: %s mcd parameter must be integral", name);
		      vpi_printf(", got vpiType=%d\n", type);
		      vpi_free_object(argv);
		      return 0;
	      }
	      value.format = vpiIntVal;
	      vpi_get_value(item, &value);
	      info->mcd = value.value.integer;
      } else {
	      info->mcd = 1;
      }

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
static vpiHandle *monitor_callbacks = 0;
static int monitor_scheduled = 0;
static int monitor_enabled = 1;

static PLI_INT32 monitor_cb_2(p_cb_data cb)
{
      do_display(1, &monitor_info);
      vpi_printf("\n");
      monitor_scheduled = 0;
      return 0;
}

/*
 * The monitor_cb_1 callback is called when an even occurs somewhere
 * in the simulation. All this function does is schedule the actual
 * display to occur in a ReadOnlySync callback. The monitor_scheduled
 * flag is used to allow only one monitor strobe to be scheduled.
 */
static PLI_INT32 monitor_cb_1(p_cb_data cause)
{
      struct t_cb_data cb;
      struct t_vpi_time time;

      if (monitor_enabled == 0) return 0;
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

static PLI_INT32 sys_monitor_calltf(char*name)
{
      unsigned idx;
      struct t_cb_data cb;
      struct t_vpi_time time;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle scope = vpi_handle(vpiScope, sys);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);

	/* If there was a previous $monitor, then remove the callbacks
	   related to it. */
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

	/* Make an array of handles from the argument list. */
      array_from_iterator(&monitor_info, argv);
      monitor_info.name = strdup(name);
      monitor_info.default_format = get_default_format(name);
      monitor_info.scope = scope;

	/* Attach callbacks to all the parameters that might change. */
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
		case vpiIntegerVar:
		case vpiRealVar:
		    /* Monitoring reg and net values involves setting
		       a callback for value changes. pass the storage
		       pointer for the callback itself as user_data so
		       that the callback can refresh itself. */
		  cb.user_data = (char*)(monitor_callbacks+idx);
		  cb.obj = monitor_info.items[idx];
		  monitor_callbacks[idx] = vpi_register_cb(&cb);
		  break;

	    }
      }

	/* When the $monitor is called, it schedules a first display
	   for the end of the current time, like a $strobe. */
      monitor_cb_1(0);

      return 0;
}

static PLI_INT32 sys_monitoron_calltf(char*name)
{
      monitor_enabled = 1;
      monitor_cb_1(0);
      return 0;
}

static PLI_INT32 sys_monitoroff_calltf(char*name)
{
      monitor_enabled = 0;
      return 0;
}

/* Implement $fdisplay and $fwrite.
 * Perhaps this could be merged into sys_display_calltf.
 */
static PLI_INT32 sys_fdisplay_calltf(char *name)
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
      switch (type) {
	    case vpiReg:
	    case vpiRealVar:
	    case vpiIntegerVar:
	    case vpiNet:
	      break;

	  case vpiConstant:
	    switch (vpi_get(vpiConstType, item)) {
		case vpiDecConst:
		case vpiBinaryConst:
		case vpiOctConst:
		case vpiHexConst:
		  break;
		default:
		  vpi_printf("ERROR: %s mcd parameter must be integral", name);
		  vpi_printf(", got vpiType=vpiConstant, vpiConstType=%d\n",
			     vpi_get(vpiConstType, item));
		  vpi_free_object(argv);
		  return 0;
	    }
	    break;

	    default:
	      vpi_printf("ERROR: %s mcd parameter must be integral", name);
	      vpi_printf(", got vpiType=%d\n", type);
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
	    my_mcd_printf(mcd, "\n");

      return 0;
}

static PLI_INT32 sys_timeformat_compiletf(char *xx)
{
      vpiHandle sys   = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv  = vpi_iterate(vpiArgument, sys);
      vpiHandle tmp;

      assert(argv);
      tmp = vpi_scan(argv);
      assert(tmp);
      assert(vpi_get(vpiType, tmp) == vpiConstant);

      tmp = vpi_scan(argv);
      assert(tmp);
      assert(vpi_get(vpiType, tmp) == vpiConstant);

      tmp = vpi_scan(argv);
      assert(tmp);
      assert(vpi_get(vpiType, tmp) == vpiConstant);

      return 0;
}

static PLI_INT32 sys_timeformat_calltf(char *xx)
{
      s_vpi_value value;
      vpiHandle sys   = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv  = vpi_iterate(vpiArgument, sys);
      vpiHandle units = vpi_scan(argv);
      vpiHandle prec  = vpi_scan(argv);
      vpiHandle suff  = vpi_scan(argv);
      vpiHandle wid   = vpi_scan(argv);

      vpi_free_object(argv);

      value.format = vpiIntVal;
      vpi_get_value(units, &value);
      timeformat_info.units = value.value.integer;

      value.format = vpiIntVal;
      vpi_get_value(prec, &value);
      timeformat_info.prec = value.value.integer;

      value.format = vpiStringVal;
      vpi_get_value(suff, &value);
      timeformat_info.suff = strdup(value.value.str);

      value.format = vpiIntVal;
      vpi_get_value(wid, &value);
      timeformat_info.width = value.value.integer;

      return 0;
}

static char *pts_convert(int value)
{
      char *string;
      switch (value) {
            case   0: string = "1s";    break;
            case  -1: string = "100ms"; break;
            case  -2: string = "10ms";  break;
            case  -3: string = "1ms";   break;
            case  -4: string = "100us"; break;
            case  -5: string = "10us";  break;
            case  -6: string = "1us";   break;
            case  -7: string = "100ns"; break;
            case  -8: string = "10ns";  break;
            case  -9: string = "1ns";   break;
            case -10: string = "100ps"; break;
            case -11: string = "10ps";  break;
            case -12: string = "1ps";   break;
            case -13: string = "100fs"; break;
            case -14: string = "10fs";  break;
            case -15: string = "1fs";   break;
            default: assert(0);
      }
      return string;
}

static PLI_INT32 sys_printtimescale_calltf(char *xx)
{
      vpiHandle sys   = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv  = vpi_iterate(vpiArgument, sys);
      vpiHandle scope = 0;
      if (!argv) {
            vpiHandle parent = vpi_handle(vpiScope, sys);
            while (parent) {
                   scope = parent;
                   parent = vpi_handle(vpiScope, scope);
            }
      } else {
            scope = vpi_scan(argv);
            vpi_free_object(argv);
      }
      
      vpi_printf("Time scale of (%s) is ", vpi_get_str(vpiFullName, scope));
      vpi_printf("%s / ", pts_convert(vpi_get(vpiTimeUnit, scope)));
      vpi_printf("%s\n", pts_convert(vpi_get(vpiTimePrecision, scope)));

      return 0;
}

static PLI_INT32 sys_end_of_compile(p_cb_data cb_data)
{
	/* The default timeformat prints times in unit of simulation
	   precision. */
      timeformat_info.suff  = strdup("");
      timeformat_info.units = vpi_get(vpiTimePrecision, 0);
      timeformat_info.prec  = 0;
      timeformat_info.width = 20;
      return 0;
}

void sys_display_register()
{
      s_cb_data cb_data;
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

      //============================== fstrobe
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fstrobe";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fstrobe";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fstrobeh";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fstrobeh";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fstrobeo";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fstrobeo";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fstrobeb";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fstrobeb";
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

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitoron";
      tf_data.calltf    = sys_monitoron_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitoron";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitoroff";
      tf_data.calltf    = sys_monitoroff_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitoroff";
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

	//============================ timeformat
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$timeformat";
      tf_data.calltf    = sys_timeformat_calltf;
      tf_data.compiletf = sys_timeformat_compiletf;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$printtimescale";
      tf_data.calltf    = sys_printtimescale_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      vpi_register_systf(&tf_data);

      cb_data.reason = cbEndOfCompile;
      cb_data.cb_rtn = sys_end_of_compile;
      cb_data.user_data = "system";
      vpi_register_cb(&cb_data);
}
