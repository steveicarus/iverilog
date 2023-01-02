/*
 * Copyright (C) 2011-2021 Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

# include <math.h>
# include <stdlib.h>
# include <string.h>
# include "config.h"
# include "vlog95_priv.h"

/*
 * Extract an int32_t value from the given bit information. If the result
 * type is 0 then the returned value is valid. If it is positive then the
 * value was too large and if it is negative then the value had undefined
 * bits. -2 is all bits z and -3 is all bits x.
 */
static int32_t get_int32_from_bits(const char *bits, unsigned nbits,
                                   unsigned is_signed, int *result_type)
{
      unsigned trim_wid = nbits - 1;
      const char msb = is_signed ? bits[trim_wid] : '0';
      unsigned idx;
      int32_t value = 0;
	/* Trim any duplicate bits from the MSB. */
      for (/* none */; trim_wid > 0; trim_wid -= 1) {
	    if (msb != bits[trim_wid]) {
		  trim_wid += 1;
		  break;
	    }
      }
      if (trim_wid < nbits) trim_wid += 1;
	/* Check to see if the value is too large. */
      if (trim_wid > 32U) {
	    *result_type = trim_wid;
	    return 0;
      }
	/* Now build the value from the bits. */
      for (idx = 0; idx < trim_wid; idx += 1) {
	    if (bits[idx] == '1') value |= (int32_t)1 << idx;
	    else if (bits[idx] != '0') {
		  *result_type = -1;
		    /* If the value is entirely x/z then return -2 or -3. */
		  if (trim_wid == 1) {
			 if (bits[idx] == 'x') *result_type -= 1;
			 *result_type -= 1;
		  }
		  return 0;
	    }
      }
	/* Sign extend as needed. */
// HERE: Need to emit 1 instead of -1 for some of the constants.
//      if (is_signed && (nbits > 1) && (msb == '1') && (trim_wid < 32U)) {
      if (is_signed && (msb == '1') && (trim_wid < 32U)) {
	    value |= ~(((uint32_t)1 << trim_wid) - (uint32_t)1);
      }
      *result_type = 0;
      return value;
}

/* Emit the given bits as either a signed or unsigned constant. If the
 * bits contain an undefined value then emit them as a binary constant
 * otherwise emit them as a hex constant. */
static void emit_bits(const char *bits, unsigned nbits, unsigned is_signed)
{
      unsigned has_undef = 0;

      assert(nbits > 0);
	/* Check for an undefined bit. */
      for (int idx = (int)nbits-1; idx >= 0; idx -= 1) {
	    if ((bits[idx] != '0') && (bits[idx] != '1')) {
		  has_undef = 1;
		  break;
	    }
      }

      fprintf(vlog_out, "%u'", nbits);
      if (is_signed) fprintf(vlog_out, "s");

	/* Emit as a binary constant. */
      if (has_undef || (nbits < 2)) {
	    int start = nbits - 1;
	    char sbit = bits[start];
	      /* Trim extra leading bits. */
	    if (! is_signed && (sbit == '1')) sbit = ' ';
	    while (start && (sbit == bits[start-1])) start -= 1;
	      /* Print the trimmed value. */
	    fprintf(vlog_out, "b");
	    for (int idx = start; idx >= 0; idx -= 1) {
		  fprintf(vlog_out, "%c", bits[idx]);
	    }
	/* Emit as a hex constant. */
      } else {
	    unsigned start = 4*(nbits/4);
	    unsigned result = 0;
	    fprintf(vlog_out, "h");
	    /* The first digit may not be a full hex digit. */
	    if (start < nbits) {
		  for (unsigned idx = start; idx < nbits; idx += 1) {
			if (bits[idx] == '1') result |= 1U << (idx%4);
		  }
		  fprintf(vlog_out, "%1x", result);
	    }
	    /* Now print the full hex digits. */
	    for (int idx = start-1; idx >= 0; idx -= 4) {
		  result = 0;
		  if (bits[idx] == '1') result |= 0x8;
		  if (bits[idx-1] == '1') result |= 0x4;
		  if (bits[idx-2] == '1') result |= 0x2;
		  if (bits[idx-3] == '1') result |= 0x1;
		  fprintf(vlog_out, "%1x", result);
	    }
      }
}

void emit_number(const char *bits, unsigned nbits, unsigned is_signed,
                 const char *file, unsigned lineno)
{
	/* If the user is allowing signed constructs then we can emit a
	 * signed number as a normal integer or with the 's syntax if
	 * an integer is not appropriate. */
      if (is_signed && allow_signed) {
	    int rtype;
	    int32_t value = get_int32_from_bits(bits, nbits, 1, &rtype);
	    if (rtype != 0) emit_bits(bits, nbits, is_signed);
	    else fprintf(vlog_out, "%"PRId32, value);
	/* Otherwise a signed value can only be 32 bits long since it can
	 * only be represented as an integer. We can trim any matching MSB
	 * bits to make it fit. We cannot support individual undefined
	 * bits in the constant. */
      } else if (is_signed) {
	    int rtype;
	    int32_t value = get_int32_from_bits(bits, nbits, 1, &rtype);
	    if (rtype > 0) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Signed number is "
		                  "greater than 32 bits (%d) and cannot be "
		                  "safely represented.\n", file, lineno,
		                  rtype);
		  vlog_errors += 1;
	    } else if (rtype == -1) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Signed number has "
		                  "an undefined bit and cannot be "
		                  "represented.\n", file, lineno);
		  vlog_errors += 1;
		  return;
	    } else if (rtype == -2) {
		  fprintf(vlog_out, "%u'bz", nbits);
	    } else if (rtype == -3) {
		    /* If this is a 32-bit wide constant then generate the
		     * undefined with integers to get a signed value. */
		  if (nbits == 32) fprintf(vlog_out, "1/0");
		  else fprintf(vlog_out, "%u'bx", nbits);
	    } else  {
		  fprintf(vlog_out, "%"PRId32, value);
	    }
	/* An unsigned number is represented in hex if all the bits are
	 * defined and it is more than a single bit otherwise it is
	 * represented in binary form to preserve all the information. */
      } else {
	    emit_bits(bits, nbits, is_signed);
      }
}

void emit_real_number(double value)
{
	/* Check for NaN. */
      if (isnan(value)) {
	    fprintf(vlog_out, "(0.0/0.0)");
	    return;
      }
	/* Check for the infinities. */
      if (isinf(value)) {
	    if (signbit(value)) fprintf(vlog_out, "(-1.0/0.0)");
	    else fprintf(vlog_out, "(1.0/0.0)");
	    return;
      }
	/* Check for +/- zero. */
      if (value == 0.0) {
	    if (signbit(value)) fprintf(vlog_out, "-0.0");
	    else fprintf(vlog_out, "0.0");
      } else {
	    char buffer[32];
	    char *cptr;
	    unsigned len;
	      /* Print the double to a temporary string using an extra digit. */
	    buffer[sizeof(buffer)-1] = 0;
	    snprintf(buffer, sizeof(buffer), "%#.17g", value);
	    assert(buffer[sizeof(buffer)-1] == 0);
	      /* Check to see if there is a digit after the decimal point and
	       * add a digit if it is missing. */
	    len = strlen(buffer);
	    if (buffer[len-1] == '.') {
		  assert((len + 1) < sizeof(buffer));
		  buffer[len] = '0';
		  len += 1;
		  buffer[len] = 0;
	    }
	      /* Now trim any extra trailing zero digits. */
	    cptr = buffer + len - 1;
	    while ((*cptr == '0') && (*(cptr-1) != '.')) cptr -= 1;
	    *(cptr+1) = 0;

	      /* Now print the processed output. */
	    fprintf(vlog_out, "%s", buffer);
      }
}

/*
 * Extract an uint64_t value from the given number expression. If the result
 * type is 0 then the returned value is valid. If it is positive then the
 * value was too large and if it is negative then the value had undefined
 * bits.
 */
uint64_t get_uint64_from_number(ivl_expr_t expr, int *result_type)
{
      unsigned nbits = ivl_expr_width(expr);
      unsigned trim_wid = nbits - 1;
      const char *bits = ivl_expr_bits(expr);
      unsigned idx;
      uint64_t value = 0;
      assert(ivl_expr_type(expr) == IVL_EX_NUMBER);
      assert(! ivl_expr_signed(expr));
	/* Trim any '0' bits from the MSB. */
      for (/* none */; trim_wid > 0; trim_wid -= 1) {
	    if ('0' != bits[trim_wid]) {
		  trim_wid += 1;
		  break;
	    }
      }
      if (trim_wid < nbits) trim_wid += 1;
	/* Check to see if the value is too large. */
      if (trim_wid > 64U) {
	    *result_type = trim_wid;
	    return 0;
      }
	/* Now build the value from the bits. */
      for (idx = 0; idx < trim_wid; idx += 1) {
	    if (bits[idx] == '1') value |= (uint64_t)1 << idx;
	    else if (bits[idx] != '0') {
		  *result_type = -1;
		    /* If the value is entirely x/z then return -2 or -3. */
		  if (trim_wid == 1) {
			 if (bits[idx] == 'x') *result_type -= 1;
			 *result_type -= 1;
		  }
		  return 0;
	    }
      }
      *result_type = 0;
      return value;
}

/*
 * Extract an int64_t value from the given number expression. If the result
 * type is 0 then the returned value is valid. If it is positive then the
 * value was too large and if it is negative then the value had undefined
 * bits. -2 is all bits z and -3 is all bits x.
 */
int64_t get_int64_from_number(ivl_expr_t expr, int *result_type)
{
      unsigned is_signed = ivl_expr_signed(expr);
      unsigned nbits = ivl_expr_width(expr);
      unsigned trim_wid = nbits - 1;
      const char *bits = ivl_expr_bits(expr);
      const char msb = is_signed ? bits[trim_wid] : '0';
      unsigned idx;
      int64_t value = 0;
      assert(ivl_expr_type(expr) == IVL_EX_NUMBER);
	/* Trim any duplicate bits from the MSB. */
      for (/* none */; trim_wid > 0; trim_wid -= 1) {
	    if (msb != bits[trim_wid]) {
		  trim_wid += 1;
		  break;
	    }
      }
      if (trim_wid < nbits) trim_wid += 1;
	/* Check to see if the value is too large. */
      if (trim_wid > 64U) {
	    *result_type = trim_wid;
	    return 0;
      }
	/* Now build the value from the bits. */
      for (idx = 0; idx < trim_wid; idx += 1) {
	    if (bits[idx] == '1') value |= (int64_t)1 << idx;
	    else if (bits[idx] != '0') {
		  *result_type = -1;
		    /* If the value is entirely x/z then return -2 or -3. */
		  if (trim_wid == 1) {
			 if (bits[idx] == 'x') *result_type -= 1;
			 *result_type -= 1;
		  }
		  return 0;
	    }
      }
	/* Sign extend as needed. */
      if (is_signed && (msb == '1') && (trim_wid < 64U)) {
	    value |= ~(((uint64_t)1 << trim_wid) - (uint64_t)1);
      }
      *result_type = 0;
      return value;
}

/*
 * Extract an int32_t value from the given number expression. If the result
 * type is 0 then the returned value is valid. If it is positive then the
 * value was too large and if it is negative then the value had undefined
 * bits. -2 is all bits z and -3 is all bits x.
 */
int32_t get_int32_from_number(ivl_expr_t expr, int *result_type)
{
      assert(ivl_expr_type(expr) == IVL_EX_NUMBER);
      return get_int32_from_bits(ivl_expr_bits(expr), ivl_expr_width(expr),
                                 ivl_expr_signed(expr), result_type);
}

/*
 * Routine to remove two characters starting at the given address.
 */
static void remove_two_chars(char* str)
{
      for (; str[2]; str += 1) {
	    str[0] = str[2];
      }
      str[0] = 0;
}

/*
 * Routine to print a string value as a string after removing any leading
 * escaped NULL bytes.
 */
void emit_string(const char* string)
{
      char *buffer = strdup(string);
      char *cptr;
      fprintf(vlog_out, "\"");
      for (cptr = buffer; *cptr; cptr += 1) {
	    if (*cptr == '\\') {
		    /* Replace any \011 with \t */
		  if ((cptr[1] == '0') && (cptr[2] == '1') &&
		      (cptr[3] == '1')) {
			cptr[1] = 't';
			remove_two_chars(cptr+2);
			cptr += 1;
		    /* Replace any \012 with \n */
		  } else if ((cptr[1] == '0') && (cptr[2] == '1') &&
		             (cptr[3] == '2')) {
			cptr[1] = 'n';
			remove_two_chars(cptr+2);
			cptr += 1;
		    /* Replace any \042 with \" */
		  } else if ((cptr[1] == '0') && (cptr[2] == '4') &&
		             (cptr[3] == '2')) {
			cptr[1] = '"';
			remove_two_chars(cptr+2);
			cptr += 1;
		    /* Replace any \134 with \\ */
		  } else if ((cptr[1] == '1') && (cptr[2] == '3') &&
		             (cptr[3] == '4')) {
			cptr[1] = '\\';
			remove_two_chars(cptr+2);
			cptr += 1;
		  } else cptr += 3;
	    }
      }
      if (*buffer) fprintf(vlog_out, "%s", buffer);
      free(buffer);
      fprintf(vlog_out, "\"");
}
