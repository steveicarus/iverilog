/*
 * Copyright (c) 2006-2021 Stephen Williams (steve@icarus.com)
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

/* round() is ISO C99 from math.h. This define should enable it. */
# define _ISOC99_SOURCE 1
# define _SVID_SOURCE 1
# define _DEFAULT_SOURCE 1

# include  "sys_priv.h"
# include  <ctype.h>
# include  <errno.h>
# include  <string.h>
# include  <stdio.h>
# include  <stdlib.h>
# include  <math.h>
# include  <limits.h>
# include  <assert.h>
# include  "ivl_alloc.h"

/*
 * The wrapper routines below get a value from either a string or a file.
 * This structure is used to determine which one is used. The unused one
 * must be assigned a zero value.
 */
struct byte_source {
      const char *str;
      FILE *fd;
};

/*
 * Wrapper routine to get a byte from either a string or a file descriptor.
 */
static int byte_getc(struct byte_source *src)
{
      if (src->str) {
	    assert(! src->fd);
	    if (src->str[0] == 0) return EOF;

	    return *(src->str)++;
      }

      assert(src->fd);
      return fgetc(src->fd);
}

/*
 * Wrapper routine to unget a byte to either a string or a file descriptor.
 */
static void byte_ungetc(struct byte_source *src, int ch)
{
      if (ch == EOF) return;

      if (src->str) {
	    assert(! src->fd);
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
 * Do we need support for +-Inf and NaN? It's not in the standard.
 *
 * The match variable is set to 1 for a match or 0 for no match.
 */
static double get_float(struct byte_source *src, unsigned width, int *match)
{
      char *endptr;
      char *strval = malloc(1);
      unsigned len = 0;
      double result;
      int ch;

	/* Skip any leading space. */
      ch = byte_getc(src);
      while (isspace(ch)) ch = byte_getc(src);

	/* If we are being asked for no digits then return a match fail. */
      if (width == 0) {
	    byte_ungetc(src, ch);
	    free(strval);
	    *match = 0;
	    return 0.0;
      }

	/* Look for the optional sign. */
      if ((ch == '+') || (ch == '-')) {
	      /* If we have a sign then the width must not be
	       * one since we need a sign and a digit. */
	    if (width == 1) {
		  byte_ungetc(src, ch);
		  free(strval);
		  *match = 0;
		  return 0.0;
	    }
	    strval = realloc(strval, len+2);
	    strval[len++] = ch;
	    ch = byte_getc(src);
      }

	/* Get any digits before the optional decimal point, but no more
	 * than width. */
      while (isdigit(ch) && (len < width)) {
	    strval = realloc(strval, len+2);
	    strval[len++] = ch;
	    ch = byte_getc(src);
      }

	/* Get the optional decimal point and any following digits, but
	 * no more than width total characters are copied. */
      if ((ch == '.') && (len < width)) {
	    strval = realloc(strval, len+2);
	    strval[len++] = ch;
	    ch = byte_getc(src);
	      /* Get any trailing digits. */
	    while (isdigit(ch) && (len < width)) {
		  strval = realloc(strval, len+2);
		  strval[len++] = ch;
		  ch = byte_getc(src);
	    }
      }

	/* No leading digits were matched. */
      if ((len == 0) ||
          ((len == 1) && ((strval[0] == '+') || (strval[0] == '-')))) {
	    byte_ungetc(src, ch);
	    free(strval);
	    *match = 0;
	    return 0.0;
      }

	/* Match an exponent. */
      if (((ch == 'e') || (ch == 'E')) && (len < width)) {
	    strval = realloc(strval, len+2);
	    strval[len++] = ch;
	    ch = byte_getc(src);

	      /* We must have enough space for at least one digit after
	       * the exponent. */
	    if (len == width) {
		  byte_ungetc(src, ch);
		  free(strval);
		  *match = 0;
		  return 0.0;
	    }

	      /* Check to see if the exponent has a sign. */
	    if ((ch == '-') || (ch == '+')) {
		  strval = realloc(strval, len+2);
		  strval[len++] = ch;
		  ch = byte_getc(src);
		    /* We must have enough space for at least one digit
		     * after the exponent sign. */
		  if (len == width) {
			byte_ungetc(src, ch);
			free(strval);
			*match = 0;
			return 0.0;
		  }
	    }

	      /* We must have at least one digit after the exponent/sign. */
	    if (! isdigit(ch)) {
		  byte_ungetc(src, ch);
		  free(strval);
		  *match = 0;
		  return 0.0;
	    }

	      /* Get the exponent digits, but no more than width total
	       * characters are copied. */
	    while (isdigit(ch) && (len < width)) {
		  strval = realloc(strval, len+2);
		  strval[len++] = ch;
		  ch = byte_getc(src);
	    }
      }
      strval[len] = 0;

	/* Put the last character back. */
      byte_ungetc(src, ch);

	/* Calculate and return the result. */
      result = strtod(strval, &endptr);
	/* If this asserts then there is a bug in the code above.*/
      assert(*endptr == 0);
      free(strval);
      *match = 1;
      return result;
}

/*
 * Routine to return a floating point value (implements %e, %f and %g).
 *
 * Return: 1 for a match, 0 for no match/variable and -1 for a
 *         suppressed match. No variable is fatal.
 */
static int scan_format_float(vpiHandle callh, vpiHandle argv,
                             struct byte_source *src, unsigned width,
                             unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name,
                             char code)
{
      vpiHandle arg;
      int match;
      s_vpi_value val;
      double result;

	/* Get the real value. */
      result = get_float(src, width, &match);

	/* Nothing was matched. */
      if (match == 0) return 0;

	/* If this match is being suppressed then return after consuming
	 * the digits and report that no arguments were used. */
      if (suppress_flag) return -1;

	/* We must have a variable to put the double value into. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() ran out of variables for %%%c format code.",
	               name, code);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Put the value into the variable. */
      val.format = vpiRealVal;
      val.value.real = result;
      vpi_put_value(arg, &val, 0, vpiNoDelay);

	/* We always consume one variable if it is available. */
      return 1;
}

/*
 * Routine to return a floating point value scaled and rounded to the
 * current time scale and precision (implements %t).
 *
 * Return: 1 for a match, 0 for no match/variable and -1 for a
 *         suppressed match. No variable is fatal.
 */
static int scan_format_float_time(vpiHandle callh, vpiHandle argv,
				  struct byte_source*src, unsigned width,
                                  unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle scope = vpi_handle(vpiScope, callh);
      int time_units = vpi_get(vpiTimeUnit, scope);
      vpiHandle arg;
      int match;
      s_vpi_value val;
      double result;
      double scale;

	/* Get the raw real value. */
      result = get_float(src, width, &match);

	/* Nothing was matched. */
      if (match == 0) return 0;

	/* If this match is being suppressed then return after consuming
	 * the digits and report that no arguments were used. */
      if (suppress_flag) return -1;

	/* Round the raw value to the specified precision. Handle this
	   by shifting the decimal point to the precision where we want
	   to round, do the rounding, then shift the decimal point back */
      scale = pow(10.0, timeformat_info.prec);
      result = round(result*scale) / scale;

	/* Scale the value from the timeformat units to the current
	 * timescale units. To minimize the error keep the scale an
	 * integer value. */
      if (timeformat_info.units >= time_units) {
	    scale = pow(10.0, timeformat_info.units - time_units);
	    result *= scale;
      } else {
	    scale = pow(10.0, time_units - timeformat_info.units);
	    result /= scale;
      }

	/* We must have a variable to put the double value into. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() ran out of variables for %%t format code.", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Put the value into the variable. */
      val.format = vpiRealVal;
      val.value.real = result;
      vpi_put_value(arg, &val, 0, vpiNoDelay);

	/* We always consume one variable if it is available. */
      return 1;
}

/*
 * Base routine for getting binary, octal and hex values.
 *
 * Return: 1 for a match, 0 for no match/variable and -1 for a
 *         suppressed match. No variable is fatal.
 */
static int scan_format_base(vpiHandle callh, vpiHandle argv,
                            struct byte_source *src, unsigned width,
                            unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name,
                            const char *match, char code,
                            PLI_INT32 type)
{
      vpiHandle arg;
      char *strval = malloc(1);
      unsigned len = 0;
      s_vpi_value val;
      int ch;

	/* Skip any leading space. */
      ch = byte_getc(src);
      while (isspace(ch)) ch = byte_getc(src);

	/* If we are being asked for no digits or the first character is
	 * an underscore then return a match fail. */
      if ((width == 0) || (ch == '_')) {
	    byte_ungetc(src, ch);
	    free(strval);
	    return 0;
      }

	/* Get all the digits, but no more than width. */
      while (strchr(match , ch) && (len < width)) {
	    if (ch == '?') ch = 'x';

	    strval = realloc(strval, len+2);
	    strval[len++] = ch;

	    ch = byte_getc(src);
      }
      strval[len] = 0;

	/* Put the last character back. */
      byte_ungetc(src, ch);

	/* Nothing was matched. */
      if (len == 0) {
	    free(strval);
	    return 0;
      }

	/* If this match is being suppressed then return after consuming
	 * the digits and report that no arguments were used. */
      if (suppress_flag) {
	    free(strval);
	    return -1;
      }

	/* We must have a variable to put the binary value into. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() ran out of variables for %%%c format code.",
	               name, code);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    free(strval);
	    return 0;
      }

	/* Put the value into the variable. */
      val.format = type;
      val.value.str = strval;
      vpi_put_value(arg, &val, 0, vpiNoDelay);
      free(strval);

	/* We always consume one variable if it is available. */
      return 1;
}

/*
 * Routine to return a binary value (implements %b).
 */
static int scan_format_binary(vpiHandle callh, vpiHandle argv,
                              struct byte_source *src, int width,
                              unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      return scan_format_base(callh, argv, src, width, suppress_flag, name,
                              "01xzXZ?_", 'b', vpiBinStrVal);
}

/*
 * Routine to return a character value (implements %c).
 *
 * Return: 1 for a match, 0 for no match/variable and -1 for a
 *         suppressed match. No variable is fatal.
 */
static int scan_format_char(vpiHandle callh, vpiHandle argv,
                            struct byte_source *src, unsigned width,
                            unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle arg;
      s_vpi_value val;
      int ch;

	/* If we are being asked for no digits then return a match fail. */
      if (width == 0) return 0;

	/* Get the character to return. */
      ch = byte_getc(src);

	/* Nothing was matched. */
      if (ch == EOF) return 0;

	/* If this match is being suppressed then return after consuming
	 * the character and report that no arguments were used. */
      if (suppress_flag) return -1;

	/* We must have a variable to put the character value into. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() ran out of variables for %%c format code.", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Put the character into the variable. */
      val.format = vpiIntVal;
      val.value.integer = ch;
      vpi_put_value(arg, &val, 0, vpiNoDelay);

	/* We always consume one variable if it is available. */
      return 1;
}

/*
 * Routine to return a decimal value (implements %d).
 *
 * Return: 1 for a match, 0 for no match/variable and -1 for a
 *         suppressed match. No variable is fatal.
 */
static int scan_format_decimal(vpiHandle callh, vpiHandle argv,
                               struct byte_source *src, unsigned width,
                               unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle arg;
      char *strval = malloc(1);
      s_vpi_value val;
      int ch;

	/* Skip any leading space. */
      ch = byte_getc(src);
      while (isspace(ch)) ch = byte_getc(src);

	/* If we are being asked for no digits or the first character is
	 * an underscore then return a match fail. */
      if ((width == 0) || (ch == '_')) {
	    byte_ungetc(src, ch);
	    free(strval);
	    return 0;
      }

	/* A decimal can match a single x/X, ? or z/Z character. */
      if (strchr("xX?", ch)) {
	    strval = realloc(strval, 2);
	    strval[0] = 'x';
	    strval[1] = 0;
      } else if (strchr("zZ", ch)) {
	    strval = realloc(strval, 2);
	    strval[0] = 'z';
	    strval[1] = 0;
      } else {
	    unsigned len = 0;

	      /* To match a + or - we must have a digit after it. */
	    if (ch == '+') {
		    /* If we have a '+' sign then the width must not be
		     * one since we need a sign and a digit. */
		  if (width == 1) {
			free(strval);
			return 0;
		  }

		  ch = byte_getc(src);
		  if (! isdigit(ch)) {
			byte_ungetc(src, ch);
			free(strval);
			return 0;
		  }
		    /* The '+' used up a character. */
		  width -= 1;
	    } else if (ch == '-') {
		    /* If we have a '-' sign then the width must not be
		     * one since we need a sign and a digit. */
		  if (width == 1) {
			free(strval);
			return 0;
		  }

		  ch = byte_getc(src);
		  if (isdigit(ch)) {
			strval = realloc(strval, len+2);
			strval[len++] = '-';
		  } else {
			byte_ungetc(src, ch);
			free(strval);
			return 0;
		  }
	    }

	      /* Get all the characters, but no more than width. */
	    while ((isdigit(ch) || ch == '_') && (len < width)) {
		  strval = realloc(strval, len+2);
		  strval[len++] = ch;

		  ch = byte_getc(src);
	    }
	    strval[len] = 0;

	      /* Put the last character back. */
	    byte_ungetc(src, ch);

	      /* Nothing was matched. */
	    if (len == 0) {
		  free(strval);
		  return 0;
	    }
      }

	/* If this match is being suppressed then return after consuming
	 * the digits and report that no arguments were used. */
      if (suppress_flag) {
	    free(strval);
	    return -1;
      }

	/* We must have a variable to put the decimal value into. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() ran out of variables for %%d format code.", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    free(strval);
	    return 0;
      }

	/* Put the decimal value into the variable. */
      val.format = vpiDecStrVal;
      val.value.str = strval;
      vpi_put_value(arg, &val, 0, vpiNoDelay);
      free(strval);

	/* We always consume one variable if it is available. */
      return 1;
}

/*
 * Routine to return a hex value (implements %h).
 */
static int scan_format_hex(vpiHandle callh, vpiHandle argv,
                           struct byte_source *src, unsigned width,
                           unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      return scan_format_base(callh, argv, src, width, suppress_flag, name,
                              "0123456789abcdefxzABCDEFXZ?_", 'h',
                              vpiHexStrVal);
}

/*
 * Routine to return an octal value (implements %o).
 */
static int scan_format_octal(vpiHandle callh, vpiHandle argv,
                             struct byte_source *src, unsigned width,
                             unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      return scan_format_base(callh, argv, src, width, suppress_flag, name,
                              "01234567xzXZ?_", 'o', vpiOctStrVal);
}


/*
 * Routine to return the current hierarchical path (implements %m).
 */
static int scan_format_module_path(vpiHandle callh, vpiHandle argv,
                                   unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle scope, arg;
      char *module_path;
      s_vpi_value val;

	/* If this format code is being suppressed then just return that no
	 * arguments were used. */
      if (suppress_flag) return -1;

	/* We must have a variable to put the hierarchical path into. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() ran out of variables for %%m format code.", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Get the current hierarchical path. */
      scope = vpi_handle(vpiScope, callh);
      assert(scope);
      module_path = vpi_get_str(vpiFullName, scope);

	/* Put the hierarchical path into the variable. */
      val.format = vpiStringVal;
      val.value.str = module_path;
      vpi_put_value(arg, &val, 0, vpiNoDelay);

	/* We always consume one variable if it is available. */
      return 1;
}

/*
 * Routine to return a string value (implements %s).
 *
 * Return: 1 for a match, 0 for no match/variable and -1 for a
 *         suppressed match. No variable is fatal.
 */
static int scan_format_string(vpiHandle callh, vpiHandle argv,
                              struct byte_source *src, unsigned width,
                              unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle arg;
      char *strval = malloc(1);
      unsigned len = 0;
      s_vpi_value val;
      int ch;

	/* Skip any leading space. */
      ch = byte_getc(src);
      while (isspace(ch)) ch = byte_getc(src);

	/* If we are being asked for no digits then return a match fail. */
      if (width == 0) {
	    byte_ungetc(src, ch);
	    free(strval);
	    return 0;
      }

	/* Get all the non-space characters, but no more than width. */
      while (! isspace(ch) && (len < width)) {
	    if (ch == EOF) break;

	    strval = realloc(strval, len+2);
	    strval[len++] = ch;

	    ch = byte_getc(src);
      }
      strval[len] = 0;

	/* Nothing was matched (this can only happen at EOF). */
      if (len == 0) {
	    assert(ch == EOF);
	    free(strval);
	    return 0;
      }

	/* Put the last character back. */
      byte_ungetc(src, ch);

	/* If this match is being suppressed then return after consuming
	 * the string and report that no arguments were used. */
      if (suppress_flag) {
	    free(strval);
	    return -1;
      }

	/* We must have a variable to put the string into. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() ran out of variables for %%s format code.", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    free(strval);
	    return 0;
      }

	/* Put the string into the variable. */
      val.format = vpiStringVal;
      val.value.str = strval;
      vpi_put_value(arg, &val, 0, vpiNoDelay);
      free(strval);

	/* We always consume one variable if it is available. */
      return 1;
}

/*
 * Routine to return a two state value (implements %u).
 *
 * Note: Since this is a binary routine it also does not check for leading
 *       space characters or use space as a boundary character.
 *
 * Return: 1 for a match, 0 for no match/variable and -1 for a
 *         suppressed match. No variable is fatal.
 */
static int scan_format_two_state(vpiHandle callh, vpiHandle argv,
                                 struct byte_source *src, unsigned width,
                                 unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle arg;
      p_vpi_vecval val_ptr;
      s_vpi_value val;
      unsigned words, word;
      PLI_INT32 varlen;

	/* If we are being asked for no data then return a match fail. */
      if (width == 0) return 0;

	/* Since this is a binary format we do not have an ending sequence.
	 * Consequently we need to use the width to determine how many word
	 * pairs to remove from the input stream. */
      if (suppress_flag) {
	      /* If no width was given then just remove one word pair. */
	    if (width == UINT_MAX) words = 1;
	    else words = (width+31)/32;
	    for (word = 0; word < words; word += 1) {
		  unsigned byte;
		    /* For a suppression we do not care about the endian order
		     * of the bytes. */
		  for (byte = 0; byte < 4; byte += 1) {
			int ch = byte_getc(src);
			  /* See the EOF comments below for more details. */
			if (ch == EOF) {
			      vpi_printf("WARNING: %s:%d: ",
			                 vpi_get_str(vpiFile, callh),
			                 (int)vpi_get(vpiLineNo, callh));
			      vpi_printf("%s() only found %u of %u bytes "
			                 "needed by %%u format code.\n", name,
			                 word*4U + byte, words*4U);
			      return 0;
			}
		  }
	    }
	    return -1;
      }

	/* We must have a variable to put the bits into. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() ran out of variables for %%u format code.", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Extract either the given number of word pairs or enough to fill
	 * the variable. */
      varlen = (vpi_get(vpiSize, arg)+31)/32;
      assert(varlen > 0);
      val_ptr = (p_vpi_vecval) malloc(varlen*sizeof(s_vpi_vecval));
      if (width == UINT_MAX) words = (unsigned)varlen;
      else words = (width+31)/32;
      for (word = 0; word < words; word += 1) {
	    int byte;
	    PLI_INT32 bits = 0;
#ifdef WORDS_BIGENDIAN
	    for (byte = 3; byte >= 0; byte -= 1) {
#else
	    for (byte = 0; byte <= 3; byte += 1) {
#endif
		  int ch = byte_getc(src);
		    /* If there is an EOF while reading the bytes then treat
		     * that as a non-match. It could be argued that the bytes
		     * should be put back, but that is not practical and since
		     * a binary read should be treated as an atomic operation
		     * it's not helpful either. An EOF while reading is an
		     * error in the data stream so print a message to help the
		     * user debug what is going wrong. The calling routine has
		     * already verified that there is at least one byte
		     * available so this message is not printed when an EOF
		     * occurs at a format code boundary. Which may not be an
		     * error in the data stream. */
		  if (ch == EOF) {
			vpi_printf("WARNING: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			vpi_printf("%s() only found %d of %u bytes needed by "
			           "%%u format code.\n", name, word*4 +
#ifdef WORDS_BIGENDIAN
			           (3-byte),
#else
			           byte,
#endif
			           words*4U);
			free(val_ptr);
			return 0;
		  }
		  bits |= (ch & 0xff) << byte*8;
	    }
	      /* Only save the words that are in range. */
	    assert(varlen>=0);
	    if (word < (unsigned)varlen) {
		  val_ptr[word].aval = bits;
		  val_ptr[word].bval = 0;
	    }
      }

	/* Mask the upper bits to match the specified width when required. */
      if (width != UINT_MAX) {
	    PLI_INT32 mask = UINT32_MAX >> (32U - ((width - 1U) % 32U + 1U));
	    val_ptr[words-1].aval &= mask;
      }

	/* Not enough words were read to fill the variable so zero fill the
	 * upper words. */
      assert(varlen>=0);
      if (words < (unsigned)varlen) {
	    for (word = words; word < (unsigned)varlen ; word += 1) {
		  val_ptr[word].aval = 0;
		  val_ptr[word].bval = 0;
	    }
      }

	/* Put the two-state value into the variable. */
      val.format = vpiVectorVal;
      val.value.vector = val_ptr;
      vpi_put_value(arg, &val, 0, vpiNoDelay);
      free(val_ptr);

	/* One variable was consumed. */
      return 1;
}


/*
 * Routine to return a four state value (implements %z).
 *
 * Note: Since this is a binary routine it also does not check for leading
 *       space characters or use space as a boundary character.
 *
 * Return: 1 for a match, 0 for no match/variable and -1 for a
 *         suppressed match. No variable is fatal.
 */
static int scan_format_four_state(vpiHandle callh, vpiHandle argv,
                                  struct byte_source *src, unsigned width,
                                  unsigned suppress_flag, ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle arg;
      p_vpi_vecval val_ptr;
      s_vpi_value val;
      unsigned words, word;
      PLI_INT32 varlen;

	/* If we are being asked for no data then return a match fail. */
      if (width == 0) return 0;

	/* Since this is a binary format we do not have an ending sequence.
	 * Consequently we need to use the width to determine how many word
	 * pairs to remove from the input stream. */
      if (suppress_flag) {
	      /* If no width was given then just remove one word pair. */
	    if (width == UINT_MAX) words = 1;
	    else words = (width+31)/32;
	    for (word = 0; word < words; word += 1) {
		  unsigned byte;
		    /* For a suppression we do not care about the endian order
		     * of the bytes. */
		  for (byte = 0; byte < 8; byte += 1) {
			int ch = byte_getc(src);
			  /* See the EOF comments below for more details. */
			if (ch == EOF) {
			      vpi_printf("WARNING: %s:%d: ",
			                 vpi_get_str(vpiFile, callh),
			                 (int)vpi_get(vpiLineNo, callh));
			      vpi_printf("%s() only found %u of %u bytes "
			                 "needed by %%z format code.\n", name,
			                 word*8U + byte, words*8U);
			      return 0;
			}
		  }
	    }
	    return -1;
      }

	/* We must have a variable to put the bits into. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s() ran out of variables for %%z format code.", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Extract either the given number of word pairs or enough to fill
	 * the variable. */
      varlen = (vpi_get(vpiSize, arg)+31)/32;
      assert(varlen > 0);
      val_ptr = (p_vpi_vecval) malloc(varlen*sizeof(s_vpi_vecval));
      if (width == UINT_MAX) words = (unsigned)varlen;
      else words = (width+31)/32;
      for (word = 0; word < words; word += 1) {
	    unsigned elem;
	    for (elem = 0; elem < 2; elem += 1) {
		  int byte;
		  PLI_INT32 bits = 0;
#ifdef WORDS_BIGENDIAN
		  for (byte = 3; byte >= 0; byte -= 1) {
#else
		  for (byte = 0; byte <= 3; byte += 1) {
#endif
			int ch = byte_getc(src);
			  /* If there is an EOF while reading the bytes then
			   * treat that as a non-match. It could be argued that
			   * the bytes should be put back, but that is not
			   * practical and since a binary read should be
			   * treated as an atomic operation it's not helpful
			   * either. An EOF while reading is an error in the
			   * data stream so print a message to help the user
			   * debug what is going wrong. The calling routine has
			   * already verified that there is at least one byte
			   * available so this message is not printed when an
			   * EOF occurs at a format code boundary. Which may
			   * not be an error in the data stream. */
			if (ch == EOF) {
			      vpi_printf("WARNING: %s:%d: ",
			                 vpi_get_str(vpiFile, callh),
			                 (int)vpi_get(vpiLineNo, callh));
			      vpi_printf("%s() only found %d of %u bytes "
			                 "needed by %%z format code.\n", name,
			                 word*8 + elem*4 +
#ifdef WORDS_BIGENDIAN
			                 (3-byte),
#else
			                 byte,
#endif
			                 words*8U);
			      free(val_ptr);
			      return 0;
			}
			bits |= (ch & 0xff) << byte*8;
		  }
		    /* Only save the words that are in range. */
		  if (word < (unsigned)varlen) {
			*(&val_ptr[word].aval+elem) = bits;
		  }
	    }
      }

	/* Mask the upper bits to match the specified width when required. */
      if (width != UINT_MAX) {
	    PLI_INT32 mask = UINT32_MAX >> (32U - ((width - 1U) % 32U + 1U));
	    val_ptr[words-1].aval &= mask;
	    val_ptr[words-1].bval &= mask;
      }

	/* Not enough words were read to fill the variable so zero fill the
	 * upper words. */
      assert(varlen>=0);
      if (words < (unsigned)varlen) {
	    for (word = words; word < (unsigned)varlen ; word += 1) {
		  val_ptr[word].aval = 0;
		  val_ptr[word].bval = 0;
	    }
      }

	/* Put the four-state value into the variable. */
      val.format = vpiVectorVal;
      val.value.vector = val_ptr;
      vpi_put_value(arg, &val, 0, vpiNoDelay);
      free(val_ptr);

	/* One variable was consumed. */
      return 1;
}

/*
 * The $fscanf and $sscanf functions are the same except for the first
 * argument, which is the source. The wrapper functions below peel off
 * the first argument and make a byte_source object that then gets
 * passed to this function, which processes the rest of the function.
 */
static int scan_format(vpiHandle callh, struct byte_source*src, vpiHandle argv,
                       ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      s_vpi_value val;
      vpiHandle item;
      PLI_INT32 len, words, idx, mask;

      char *fmt, *fmtp;
      int rc = 0;
      int ch;

      int match = 1;

	/* Get the format string. */
      item = vpi_scan(argv);
      assert(item);
	/* Look for an undefined bit (X/Z) in the format string. If one is
	 * found just return EOF. */
      len = vpi_get(vpiSize, item);
      words = ((len + 31) / 32) - 1;
      val.format = vpiVectorVal;
      vpi_get_value(item, &val);
	/* Check the full words for an undefined bit. */
      for (idx = 0; idx < words; idx += 1) {
	    if (val.value.vector[idx].bval) {
		  match = 0;
		  rc = EOF;
		  break;
	    }
      }
	/* The mask is defined to be 32 bits. */
      mask = UINT32_MAX >> (32U - ((len - 1U) % 32U + 1U));
	/* Check the top word for an undefined bit. */
      if (match && (val.value.vector[words].bval & mask)) {
	    match = 0;
	    rc = EOF;
      }

	/* Now get the format as a string. */
      val.format = vpiStringVal;
      vpi_get_value(item, &val);
      fmtp = fmt = strdup(val.value.str);

	/* See if we are at EOF before we even start. */
      ch = byte_getc(src);
      if (ch == EOF) {
	    rc = EOF;
	    match = 0;
      }
      byte_ungetc(src, ch);

      while ( fmtp && *fmtp != 0 && match) {

	    if (isspace((int)*fmtp)) {
		    /* White space matches a string of white space in the
		     * input. The number of spaces is not relevant, and
		     * the match may be 0 or more spaces. */
		  while (*fmtp && isspace((int)*fmtp)) fmtp += 1;

		  ch = byte_getc(src);
		  while (isspace(ch)) ch = byte_getc(src);

		  byte_ungetc(src, ch);

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
		     * the format %<N>x no matter what the x code, so
		     * parse it generically first. */

		  unsigned suppress_flag = 0;
		  unsigned max_width = UINT_MAX;
		  int code = 0;

		    /* Look for the suppression character '*'. */
		  fmtp += 1;
		  if (*fmtp == '*') {
			suppress_flag = 1;
			fmtp += 1;
		  }
		    /* Look for the maximum match width. */
		  if (isdigit((int)*fmtp)) {
			max_width = 0;
			while (isdigit((int)*fmtp)) {
			      max_width *= 10;
			      max_width += *fmtp - '0';
			      fmtp += 1;
			}
		  }

		    /* Get the format character. */
		  code = *fmtp;
		  fmtp += 1;

		    /* The format string is parsed:
		     *   - max_width is the width,
		     *   - code is the format code character,
		     *   - suppress_flag is true if the match is to be ignored.
		     * Now interpret the code. */
		  switch (code) {

			  /* Read a '%' character from the input. */
		      case '%':
			assert(max_width == -1U);
			assert(suppress_flag == 0);
			ch = byte_getc(src);
			if (ch != '%') {
			      byte_ungetc(src, ch);
			      match = 0;
			}
			break;

		      case 'b':
			match = scan_format_binary(callh, argv, src, max_width,
			                           suppress_flag, name);
			if (match == 1) rc += 1;
			break;

		      case 'c':
			match = scan_format_char(callh, argv, src, max_width,
			                       suppress_flag, name);
			if (match == 1) rc += 1;
			break;

		      case 'd':
			match = scan_format_decimal(callh, argv, src, max_width,
			                            suppress_flag, name);
			if (match == 1) rc += 1;
			break;

		      case 'e':
		      case 'f':
		      case 'g':
			match = scan_format_float(callh, argv, src, max_width,
			                          suppress_flag, name, code);
			if (match == 1) rc += 1;
			break;

		      case 'h':
		      case 'x':
			match = scan_format_hex(callh, argv, src, max_width,
			                        suppress_flag, name);
			if (match == 1) rc += 1;
			break;

		      case 'm':
			  /* Since this code does not consume any characters
			   * the width makes no difference. */
			match = scan_format_module_path(callh, argv,
			                                suppress_flag, name);
			if (match == 1) rc += 1;
			break;

		      case 'o':
			match = scan_format_octal(callh, argv, src, max_width,
			                          suppress_flag, name);
			if (match == 1) rc += 1;
			break;

		      case 's':
			match = scan_format_string(callh, argv, src, max_width,
			                           suppress_flag, name);
			if (match == 1) rc += 1;
			break;

		      case 't':
			match = scan_format_float_time(callh, argv, src,
			                               max_width,
			                               suppress_flag, name);
			if (match == 1) rc += 1;
			break;

		      case 'u':
			match = scan_format_two_state(callh, argv, src,
			                              max_width,
			                              suppress_flag, name);
			  /* If a binary match fails and it is the first item
			   * matched then treat that as an EOF. */
			if ((match == 0) && (rc == 0)) rc = EOF;
			if (match == 1) rc += 1;
			break;

		      case 'v':
			vpi_printf("SORRY: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			vpi_printf("%s() format code '%%%c' is not "
			           "currently supported.\n", name, code);
			vpip_set_return_value(1);
			vpi_control(vpiFinish, 1);
			break;

		      case 'z':
			match = scan_format_four_state(callh, argv, src,
			                               max_width,
			                               suppress_flag, name);
			  /* If a binary match fails and it is the first item
			   * matched then treat that as an EOF. */
			if ((match == 0) && (rc == 0)) rc = EOF;
			if (match == 1) rc += 1;
			break;

		      default:
			vpi_printf("ERROR: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			vpi_printf("%s() was given an invalid format code: "
			           "%%%c\n", name, code);
			vpip_set_return_value(1);
			vpi_control(vpiFinish, 1);
			break;
		  }
	    }
      }

	/* Clean up the allocated memory. */
      free(fmt);
      vpi_free_object(argv);

	/* Return the number of successful matches. */
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
	    // fallthrough
	case vpiIntegerVar:
	case vpiBitVar:
	case vpiByteVar:
	case vpiShortIntVar:
	case vpiIntVar:
	case vpiLongIntVar:
	case vpiRealVar:
	case vpiReg:
	case vpiTimeVar:
	case vpiStringVar:
	    rtn = 1;
	    break;
      }

      return rtn;
}

static int sys_check_args(vpiHandle callh, vpiHandle argv, const PLI_BYTE8 *name)
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

static PLI_INT32 sys_fscanf_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires at least three argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The first argument must be a file descriptor. */
      if (! is_numeric_obj(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument (fd) must be numeric.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    vpi_free_object(argv);
	    return 0;
      }

      if (sys_check_args(callh, argv, name)) {
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }
      return 0;
}

static PLI_INT32 sys_fscanf_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
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
	               (int)val.value.integer, name);
	    errno = EBADF;
	    val.format = vpiIntVal;
	    val.value.integer = EOF;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    vpi_free_object(argv);
	    return 0;
      }

      src.str = 0;
      src.fd = fd;
      scan_format(callh, &src, argv, name);

      return 0;
}

static PLI_INT32 sys_sscanf_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle reg;

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires at least three argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The first argument must be a register or a string. */
      reg = vpi_scan(argv);  /* This should never be zero. */
      switch(vpi_get(vpiType, reg)) {
	  case vpiReg:
	  case vpiStringVar:
	    break;
	  case vpiConstant:
	  case vpiParameter:
	    if (vpi_get(vpiConstType, reg) == vpiStringConst) break;
	    // fallthrough
	  default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument must be a register or a string.\n",
	               name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    vpi_free_object(argv);
	    return 0;
      }

      if (sys_check_args(callh, argv, name)) {
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }
      return 0;
}

static PLI_INT32 sys_sscanf_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
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
      scan_format(callh, &src, argv, name);
      free(str);

      return 0;
}

void sys_scanf_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      /*============================== fscanf */
      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname      = "$fscanf";
      tf_data.calltf      = sys_fscanf_calltf;
      tf_data.compiletf   = sys_fscanf_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$fscanf";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== sscanf */
      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname      = "$sscanf";
      tf_data.calltf      = sys_sscanf_calltf;
      tf_data.compiletf   = sys_sscanf_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$sscanf";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
