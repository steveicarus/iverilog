/*
 * Copyright (c) 1999-2021 Stephen Williams (steve@icarus.com)
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
# include  <assert.h>
# include  <string.h>
# include  <errno.h>
# include  <ctype.h>
# include  <stdio.h>
# include  <stdlib.h>
# include  <math.h>
# include  "ivl_alloc.h"

// Flag to enable better compatibility with other simulators
static unsigned compatible_flag = 0;

static void check_command_line_args(void)
{
      struct t_vpi_vlog_info vlog_info;

      vpi_get_vlog_info(&vlog_info);

      for (int idx = 0 ;  idx < vlog_info.argc ;  idx += 1) {
	    if (strcmp(vlog_info.argv[idx],"-compatible") == 0) {
		  compatible_flag = 1;

	    }
      }
}

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

static void my_mcd_rawwrite(PLI_UINT32 mcd, const char*buf, size_t count)
{
      if (IS_MCD(mcd)) {
	    vpip_mcd_rawwrite(mcd, buf, count);
      } else {
	    FILE*fp = vpi_get_file(mcd);
	    if (fp) {
		  while (count > 0) {
			size_t rc = fwrite(buf, 1, count, fp);
			if (rc == 0) break;
			count -= rc;
			buf += rc;
		  }
	    }
      }
}

struct timeformat_info_s timeformat_info = { 0, 0, 0, 20 };

struct strobe_cb_info {
      const char*name;
      char*filename;
      int lineno;
      int default_format;
      vpiHandle scope;
      vpiHandle*items;
      unsigned nitems;
      unsigned fd_mcd;
};

/*
 * The number of decimal digits needed to represent a
 * nr_bits binary number is floor(nr_bits*log_10(2))+1,
 * where log_10(2) = 0.30102999566398....  and I approximate
 * this transcendental number as 146/485, to avoid the vagaries
 * of floating-point.  The smallest nr_bits for which this
 * approximation fails is 2621,
 * 2621*log_10(2)=789.9996, but (2621*146+484)/485=790 (exactly).
 * In cases like this, all that happens is we allocate one
 * unneeded char for the output.  I add a "L" suffix to 146
 * to make sure the computation is done as long ints, otherwise
 * on a 16-bit int machine (allowed by ISO C) we would mangle
 * this computation for bit-length of 224.  I'd like to put
 * in a test for nr_bits < LONG_MAX/146, but don't know how
 * to fail, other than crashing.
 *
 * In an April 2000 thread in comp.unix.programmer, with subject
 * "integer -> string", I <LRDoolittle@lbl.gov> give the 28/93
 * approximation, but overstate its accuracy: that version first
 * fails when the number of bits is 289, not 671.
 *
 * This result does not include space for a trailing '\0', if any.
*/
__inline__ static int calc_dec_size(int nr_bits, int is_signed)
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

static int get_default_format(const char *name)
{
    int default_format;

    switch(name[ strlen(name)-1 ]){
	/*  writE/strobE or monitoR or displaY/fdisplaY or sformaT/sformatF */
    case 'e':
    case 'r':
    case 't':
    case 'f':
    case 'y': default_format = vpiDecStrVal; break;
    case 'h': default_format = vpiHexStrVal; break;
    case 'o': default_format = vpiOctStrVal; break;
    case 'b': default_format = vpiBinStrVal; break;
    default:
	default_format = -1;
	assert(0);
    }

    return default_format;
}

/* Build the format using the variables that control how the item will
 * be printed. This is used in error messages and directly by the e/f/g
 * format codes (minus the enclosing <>). The user needs to free the
 * returned string. */
static char * format_as_string(int ljust, int plus, int ld_zero, int width,
                               int prec, char fmt)
{
  char buf[256];
  unsigned int size = 0;

  /* Do not remove/change the "<" without also changing the e/f/g format
   * code below! */
  buf[size++] = '<';
  buf[size++] = '%';
  if (ljust == 1) buf[size++] = '-';
  if (plus == 1) buf[size++] = '+';
  if (ld_zero == 1) buf[size++] = '0';
  if (width != -1)
    size += sprintf(&buf[size], "%d", width);
  if (prec != -1)
    size += sprintf(&buf[size], ".%d", prec);
  if (fmt) buf[size++] = fmt;
  /* The same goes here ">"! */
  buf[size++] = '>';
  buf[size] = '\0';
  return strdup(buf);
}

static void get_time(char *rtn, const char *value, int prec,
                     PLI_INT32 time_units)
{
  int shift = time_units - timeformat_info.units;

  /* Strip any leading zeros, but leave a single zero. */
  while (value[0] == '0' && value[1] != '\0') value += 1;
  /* We need to scale the number up. */
  if (shift >= 0) {
    strcpy(rtn, value);
    /* Shift only non-zero values. */
    while (shift > 0 && value[0] != '0') {
      strcat(rtn, "0");
      shift -= 1;
    }
    if (prec > 0) strcat(rtn, ".");
    while (prec > 0) {
      strcat(rtn, "0");
      prec -= 1;
    }

  /* We need to scale the number down. */
  } else {
    int len = strlen(value);
    int head = len + shift;
    int tail;
    /* We have digits to the left of the decimal point. */
    if (head > 0) {
      strncpy(rtn, value, head);
      *(rtn+head) = '\0';
      if (prec > 0) {
        strcat(rtn, ".");
        strncat(rtn, &value[head], prec);
        tail = prec + shift;
        while (tail > 0) {
          strcat(rtn, "0");
          tail -= 1;
        }
      }
    /* All digits are to the right of the decimal point. */
    } else {
      strcpy(rtn, "0");
      if (prec > 0) strcat(rtn, ".");
      /* Add leading zeros as needed. */
      head = -head;
      if (head > prec) head = prec;
      while (head > 0) {
        strcat(rtn, "0");
        head -= 1;
      }
      /* Add digits from the value if they fit. */
      tail = prec + len + shift;
      if (tail > 0) {
        strncat(rtn, value, tail);
        /* Add trailing zeros to fill out the precision. */
        tail = prec + shift + 1 - len;
        while (tail > 0) {
          strcat(rtn, "0");
          tail -= 1;
        }
      }
    }
  }

  strcat(rtn, timeformat_info.suff);
}

static void get_time_real(char *rtn, double value, int prec,
                          PLI_INT32 time_units)
{
  /* Scale the value from its time units to the format time units. */
  if (time_units >= timeformat_info.units) {
    value *= pow(10.0, time_units - timeformat_info.units);
  } else {
    value /= pow(10.0, timeformat_info.units - time_units);
  }
  sprintf(rtn, "%0.*f%s", prec, value, timeformat_info.suff);
}

static unsigned int get_format_char(char **rtn, int ljust, int plus,
                                    int ld_zero, int width, int prec,
                                    char fmt, const struct strobe_cb_info *info,
                                    unsigned int *idx)
{
  s_vpi_value value;
  char *result, *fmtb;
  unsigned int size;
  unsigned int ini_size = 512;  /* The initial size of the buffer. */

  /* Make sure the width fits in the initial buffer. */
  assert(width >= -1);
  if ((unsigned int)(width+1) > ini_size) ini_size = width + 1;

  /* The default return value is the full format. */
  result = malloc(ini_size*sizeof(char));
  fmtb = format_as_string(ljust, plus, ld_zero, width, prec, fmt);
  strcpy(result, fmtb);
  size = strlen(result) + 1; /* fallback value if errors */
  switch (fmt) {

    case '\0':
      vpi_printf("WARNING: %s:%d: a single %% at the end of format string "
                 "%s%s will be displayed as '%%'.\n",
                 info->filename, info->lineno, info->name, fmtb);
      // fallthrough
    case '%':
      if (ljust != 0  || plus != 0 || ld_zero != 0 || width != -1 ||
          prec != -1) {
        vpi_printf("WARNING: %s:%d: invalid format %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      }
      strcpy(result, "%");
      size = strlen(result) + 1;
      break;

    case 'b':
    case 'B':
    case 'o':
    case 'O':
    case 'h':
    case 'H':
    case 'x':
    case 'X':
      *idx += 1;
      if (plus != 0 || prec != -1) {
        vpi_printf("WARNING: %s:%d: invalid format %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      }
      if (*idx >= info->nitems) {
        vpi_printf("WARNING: %s:%d: missing argument for %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      } else {
        switch (fmt) {
          case 'b':
          case 'B':
            value.format = vpiBinStrVal;
            break;
          case 'o':
          case 'O':
            value.format = vpiOctStrVal;
            break;
          case 'h':
          case 'H':
          case 'x':
          case 'X':
            value.format = vpiHexStrVal;
            break;
        }
        vpi_get_value(info->items[*idx], &value);
        if (value.format == vpiSuppressVal) {
          vpi_printf("WARNING: %s:%d: incompatible value for %s%s.\n",
                     info->filename, info->lineno, info->name, fmtb);
        } else {
          unsigned swidth = strlen(value.value.str), free_flag = 0;;
          char *cp = value.value.str;

          if (ld_zero == 1) {
            /* Strip the leading zeros if a width is not given. */
            if (width == -1) while (*cp == '0' && *(cp+1) != '\0') cp++;
            /* Pad with leading zeros. */
            else if (ljust == 0 && (signed)swidth < width) {
              unsigned pad = (unsigned)width - swidth;
              cp = malloc((width+1)*sizeof(char));
              memset(cp, '0', pad);
              strcpy(cp+pad, value.value.str);
              free_flag = 1;
            /* For a left aligned value also strip the leading zeros. */
            } else if (ljust != 0) while (*cp == '0' && *(cp+1) != '\0') cp++;
          }

          /* If a width was not given, use a width of zero. */
          if (width == -1) width = 0;

          /* If the default buffer is too small, make it big enough. */
          size = strlen(cp) + 1;
          if ((signed)size < (width+1)) size = width+1;
          if (size > ini_size) result = realloc(result, size*sizeof(char));

          if (ljust == 0) sprintf(result, "%*s", width, cp);
          else sprintf(result, "%-*s", width, cp);
          if (free_flag) free(cp);
          size = strlen(result) + 1;
        }
      }
      break;

    case 'c':
    case 'C':
      *idx += 1;
      if (plus != 0 || prec != -1) {
        vpi_printf("WARNING: %s:%d: invalid format %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      }
      if (*idx >= info->nitems) {
        vpi_printf("WARNING: %s:%d: missing argument for %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      } else {
        value.format = vpiIntVal;
        vpi_get_value(info->items[*idx], &value);
        if (value.format == vpiSuppressVal) {
          vpi_printf("WARNING: %s:%d: incompatible value for %s%s.\n",
                     info->filename, info->lineno, info->name, fmtb);
        } else {
          char ch = value.value.integer;

          /* If the width is less than one then use a width of one. */
          if (width < 1) width = 1;
          size = width + 1;
          assert(size <= ini_size);

          if (ljust == 0) {
            if (width > 1) {
              memset(result, (ld_zero == 1 ? '0': ' '), width-1);
            }
            result[width-1] = ch;
          } else {
            result[0] = ch;
            if (width > 1) {
              memset(result+1, ' ', width-1);
            }
          }
          result[width] = '\0';
        }
      }
      break;

    case 'd':
    case 'D':
      *idx += 1;
      if (prec != -1) {
        vpi_printf("WARNING: %s:%d: invalid format %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      }
      if (*idx >= info->nitems) {
        vpi_printf("WARNING: %s:%d: missing argument for %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      } else {
        value.format = vpiDecStrVal;
        vpi_get_value(info->items[*idx], &value);
        if (value.format == vpiSuppressVal) {
          vpi_printf("WARNING: %s:%d: incompatible value for %s%s.\n",
                     info->filename, info->lineno, info->name, fmtb);
        } else {
          unsigned pad = 0;
          unsigned swidth = strlen(value.value.str) +
                            (value.value.str[0] == '-' ? 0 : (unsigned)plus);
          char *tbuf, *cpb, *cp = value.value.str;

          /* Allocate storage and calculate the pad if needed. */
          if (ljust == 0 && ld_zero == 1 && (signed)swidth < width) {
            tbuf = malloc((width+1)*sizeof(char));
            pad = (unsigned)width - swidth;
          } else {
            tbuf = malloc((swidth+1)*sizeof(char));
          }
          cpb = tbuf;

          /* Insert the sign if needed. */
          if (plus == 1 && *cp != '-') {
            *cpb = '+';
            cpb += 1;
          } else if (*cp == '-') {
            *cpb = '-';
            cpb += 1;
            cp += 1;
          }

          /* Now add padding if it is needed and then add the value. */
          memset(cpb, '0', pad);
          strcpy(cpb+pad, cp);

          /* If a width was not given, use the default, unless we have a
           * leading zero (width of zero). Because the width of a real in
           * Icarus is 1 the string length will set the width of a real
           * displayed using %d. */
          if (width == -1) {
            width = (ld_zero == 1) ? 0 : vpi_get_dec_size(info->items[*idx]);
          }

          /* If the default buffer is too small make it big enough. */
          size = strlen(tbuf) + 1;
          if ((signed)size < (width+1)) size = width+1;
          if (size > ini_size) result = realloc(result, size*sizeof(char));

          if (ljust == 0) sprintf(result, "%*s", width, tbuf);
          else sprintf(result, "%-*s", width, tbuf);
          free(tbuf);
          size = strlen(result) + 1;
        }
      }
      break;

    case 'e':
    case 'E':
    case 'f':
    case 'F':
    case 'g':
    case 'G':
      *idx += 1;
      if (*idx >= info->nitems) {
        vpi_printf("WARNING: %s:%d: missing argument for %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      } else {
        value.format = vpiRealVal;
        vpi_get_value(info->items[*idx], &value);
        if (value.format == vpiSuppressVal) {
          vpi_printf("WARNING: %s:%d: incompatible value for %s%s.\n",
                     info->filename, info->lineno, info->name, fmtb);
        } else {
          char *cp = fmtb;

          if (fmt == 'F') {
            while (*cp != 'F') cp++;
            *cp = 'f';
          }
          while (*cp != '>') cp++;
          *cp = '\0';

          /* If the default buffer is too small make it big enough.
           *
           * This should always give enough space. The maximum double
           * is approximately 1.8*10^308 this means we could need 310
           * characters plus the precision. We'll use 320 to give some
           * extra buffer space. The initial buffers size should work
           * for most cases, but to be safe we add the precision to
           * the maximum size (think %6.300f when passed 1.2*10^308). */
          size = width + 1;
          if (size < 320) size = 320;
          size += prec;
          if (size > ini_size) result = realloc(result, size*sizeof(char));
#if !defined(__GNUC__)
		  if (isnan(value.value.real))
			  sprintf(result, "%s", "nan");
		  else
			  sprintf(result, fmtb+1, value.value.real);
#else
          sprintf(result, fmtb+1, value.value.real);
#endif
          size = strlen(result) + 1;
        }
      }
      break;

    /* This Verilog format specifier is not currently supported!
     * vpiCell and vpiLibrary need to be implemented first. */
    case 'l':
    case 'L':
      vpi_printf("WARNING: %s:%d: %%%c currently unsupported %s%s.\n",
                 info->filename, info->lineno, fmt, info->name, fmtb);
      break;

    case 'm':
    case 'M':
      if (plus != 0 || prec != -1) {
        vpi_printf("WARNING: %s:%d: invalid format %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      }
      /* If a width was not given, use a width of zero. */
      if (width == -1) width = 0;

      {
        char *cp = vpi_get_str(vpiFullName, info->scope);
        /* If the default buffer is too small, make it big enough. */
        size = strlen(cp) + 1;
        if ((signed)size < (width+1)) size = width+1;
        if (size > ini_size) result = realloc(result, size*sizeof(char));

        if (ljust == 0) sprintf(result, "%*s", width, cp);
        else sprintf(result, "%-*s", width, cp);
      }
      size = strlen(result) + 1;
      break;

    case 's':
    case 'S':
      /* Strings are not numeric and are not zero filled, so %08s => %8s. */
      *idx += 1;
      if (plus != 0 || prec != -1) {
        vpi_printf("WARNING: %s:%d: invalid format %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      }
      if (*idx >= info->nitems) {
        vpi_printf("WARNING: %s:%d: missing argument for %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      } else {
        value.format = vpiStringVal;
        vpi_get_value(info->items[*idx], &value);
        if (value.format == vpiSuppressVal) {
          vpi_printf("WARNING: %s:%d: incompatible value for %s%s.\n",
                     info->filename, info->lineno, info->name, fmtb);
        } else {
          if (width == -1) {
            /* If all we have is a leading zero then we want a zero width. */
            if (ld_zero == 1) width = 0;
            /* Otherwise if a width was not given, use the value width. */
            else width = (vpi_get(vpiSize, info->items[*idx])+7) / 8;
          }
          /* If the default buffer is too small make it big enough. */
          size = strlen(value.value.str) + 1;
          if ((signed)size < (width+1)) size = width+1;
          if (size > ini_size) result = realloc(result, size*sizeof(char));
          if (ljust == 0) sprintf(result, "%*s", width, value.value.str);
          else sprintf(result, "%-*s", width, value.value.str);
          size = strlen(result) + 1;
        }
      }
      break;

    case 't':
    case 'T':
      *idx += 1;
      if (*idx >= info->nitems) {
        vpi_printf("WARNING: %s:%d: missing argument for %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      } else {
        PLI_INT32 type;

        /* Get the argument type and value. */
        type = vpi_get(vpiType, info->items[*idx]);
        if (((type == vpiConstant || type == vpiParameter) &&
             vpi_get(vpiConstType, info->items[*idx]) == vpiRealConst) ||
            type == vpiRealVar || (type == vpiSysFuncCall &&
             vpi_get(vpiFuncType, info->items[*idx]) == vpiRealFunc)) {
          value.format = vpiRealVal;
        } else {
          value.format = vpiDecStrVal;
        }
        vpi_get_value(info->items[*idx], &value);
        if (value.format == vpiSuppressVal) {
          vpi_printf("WARNING: %s:%d: incompatible value for %s%s.\n",
                     info->filename, info->lineno, info->name, fmtb);
        } else {
          char *tbuf, *prev_suff = 0;
          PLI_INT32 time_units = vpi_get(vpiTimeUnit, info->scope);

          if (plus != 0) {
              /* Icarus-specific extension to print out time units.
               * It is needed by vhdlpp to correctly implement time'image(). */
              PLI_INT32 time_prec = vpi_get(vpiTimePrecision, info->scope);

              /* We are about to override the suffix string set with $timeformat(),
               * therefore we need to restore it after the call. */
              prev_suff = timeformat_info.suff;

              if (time_units < -12)
                  timeformat_info.suff = strdup(" fs");
              else if (time_units < -9)
                  timeformat_info.suff = strdup(" ps");
              else if (time_units < -6)
                  timeformat_info.suff = strdup(" ns");
              else if (time_units < -3)
                  timeformat_info.suff = strdup(" us");
              else if (time_units < 0)
                  timeformat_info.suff = strdup(" ms");
              else
                  timeformat_info.suff = strdup(" s");

              /* Adjust shift for get_time(), so the number indeed matches the unit */
              time_units += (3 + (time_units % 3)) % 3 + (time_prec - time_units);
          }

          unsigned swidth, free_flag = 0;
          unsigned suff_len = strlen(timeformat_info.suff);
          char *cp;

          /* The 512 (513-1 for EOL) is more than enough for any double
           * value (309 digits plus a decimal point maximum). Because of
           * scaling this could be larger. For decimal values you can
           * have an arbitrary value so you can overflow the buffer, but
           * for now we will assume the user will use this as intended
           * (pass a time variable or the result of a time function). */
          tbuf = malloc((513+suff_len)*sizeof(char));
          if (prec == -1) prec = timeformat_info.prec;
          if (value.format == vpiRealVal) {
            get_time_real(tbuf, value.value.real, prec, time_units);
          } else {
            get_time(tbuf, value.value.str, prec, time_units);
          }
          cp = tbuf;
          swidth = strlen(tbuf);

          if (plus != 0) {
              /* Restore the previous suffix string, overridden by
               * a vhdlpp-specific $sformatf() call. */
              free(timeformat_info.suff);
              timeformat_info.suff = prev_suff;
          }

          if (ld_zero == 1) {
            /* No leading zeros are created by this conversion so just make
             * the width 0 for this case. */
            if (width == -1) width = 0;
            /* Pad with leading zeros. */
            else if (ljust == 0 && (signed)swidth < width) {
              unsigned pad = (unsigned)width - swidth;
              cp = malloc((width+1)*sizeof(char));
              memset(cp, '0', pad);
              strcpy(cp+pad, tbuf);
              free_flag = 1;
            }
          }
          if (width == -1) width = timeformat_info.width;

          /* If the default buffer is too small make it big enough. */
          size = strlen(tbuf) + 1;
          if ((signed)size < (width+1)) size = width+1;
          if (size > ini_size) result = realloc(result, size*sizeof(char));

          if (ljust == 0) sprintf(result, "%*s", width, cp);
          else sprintf(result, "%-*s", width, cp);
          if (free_flag) free(cp);
          free(tbuf);
          size = strlen(result) + 1;
        }
      }
      break;

    case 'u':
    case 'U':
      *idx += 1;
      if (ljust != 0  || plus != 0 || ld_zero != 0 || width != -1 ||
          prec != -1) {
        vpi_printf("WARNING: %s:%d: invalid format %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      }
      if (*idx >= info->nitems) {
        vpi_printf("WARNING: %s:%d: missing argument for %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      } else {
        value.format = vpiVectorVal;
        vpi_get_value(info->items[*idx], &value);
        if (value.format == vpiSuppressVal) {
          vpi_printf("WARNING: %s:%d: incompatible value for %s%s.\n",
                     info->filename, info->lineno, info->name, fmtb);
        } else {
          PLI_INT32 veclen, word, byte;
          char *cp;

          veclen = (vpi_get(vpiSize, info->items[*idx])+31)/32;
          size = veclen * 4 + 1;
          /* If the default buffer is too small, make it big enough. */
          if (size > ini_size) result = realloc(result, size*sizeof(char));
          cp = result;
          for (word = 0; word < veclen; word += 1) {
            PLI_INT32 bits = value.value.vector[word].aval &
                   ~value.value.vector[word].bval;
#ifdef WORDS_BIGENDIAN
            for (byte = 3; byte >= 0; byte -= 1) {
#else
            for (byte = 0; byte <= 3; byte += 1) {
#endif
              *cp = (bits >> byte*8) & 0xff;
              cp += 1;
            }
          }
          *cp = '\0';
        }
      }
      /* size is defined above! We can't use strlen here since this can
       * be a binary string (can contain NULLs). */
      break;

    case 'v':
    case 'V':
      *idx += 1;
      if (plus != 0 || prec != -1) {
        vpi_printf("WARNING: %s:%d: invalid format %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      }
      if (*idx >= info->nitems) {
        vpi_printf("WARNING: %s:%d: missing argument for %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      } else {
        value.format = vpiStrengthVal;
        vpi_get_value(info->items[*idx], &value);
        if (value.format == vpiSuppressVal) {
          vpi_printf("WARNING: %s:%d: incompatible value for %s%s.\n",
                     info->filename, info->lineno, info->name, fmtb);
        } else {
          char tbuf[4], *rbuf;
          PLI_INT32 nbits;
          int bit;

          /* If a width was not given use a width of zero. */
          if (width == -1) width = 0;
          nbits = vpi_get(vpiSize, info->items[*idx]);
          /* This is 4 chars for all but the last bit (strength + "_")
           * which only needs three chars (strength), but then you need
           * space for the EOS '\0', so it is just number of bits * 4. */
          size = nbits*4;
          rbuf = malloc(size*sizeof(char));
          if ((signed)size < (width+1)) size = width+1;
          if (size > ini_size) result = realloc(result, size*sizeof(char));
          strcpy(rbuf, "");
          for (bit = nbits-1; bit >= 0; bit -= 1) {
            vpip_format_strength(tbuf, &value, bit);
            strcat(rbuf, tbuf);
	    if (bit > 0) strcat(rbuf, "_");
          }
          if (ljust == 0) sprintf(result, "%*s", width, rbuf);
          else sprintf(result, "%-*s", width, rbuf);
          free(rbuf);
          size = strlen(result) + 1;
        }
      }
      break;

    case 'z':
    case 'Z':
      *idx += 1;
      size = strlen(result) + 1; /* fallback value if errors */
      if (ljust != 0  || plus != 0 || ld_zero != 0 || width != -1 ||
          prec != -1) {
        vpi_printf("WARNING: %s:%d: invalid format %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      }
      if (*idx >= info->nitems) {
        vpi_printf("WARNING: %s:%d: missing argument for %s%s.\n",
                   info->filename, info->lineno, info->name, fmtb);
      } else {
        value.format = vpiVectorVal;
        vpi_get_value(info->items[*idx], &value);
        if (value.format == vpiSuppressVal) {
          vpi_printf("WARNING: %s:%d: incompatible value for %s%s.\n",
                     info->filename, info->lineno, info->name, fmtb);
        } else {
          PLI_INT32 veclen, word, elem, bits, byte;
          char *cp;

          veclen = (vpi_get(vpiSize, info->items[*idx])+31)/32;
          size = 2 * veclen * 4 + 1;
          /* If the default buffer is too small, make it big enough. */
          if (size > ini_size) result = realloc(result, size*sizeof(char));
          cp = result;
          for (word = 0; word < veclen; word += 1) {
            /* Write the aval followed by the bval in endian order. */
            for (elem = 0; elem < 2; elem += 1) {
              bits = *(&value.value.vector[word].aval+elem);
#ifdef WORDS_BIGENDIAN
              for (byte = 3; byte >= 0; byte -= 1) {
#else
              for (byte = 0; byte <= 3; byte += 1) {
#endif
                *cp = (bits >> byte*8) & 0xff;
                cp += 1;
              }
            }
          }
          *cp = '\0';
        }
      }
      /* size is defined above! We can't use strlen here since this can
       * be a binary string (can contain NULLs). */
      break;

    default:
      vpi_printf("WARNING: %s:%d: unknown format %s%s.\n",
                 info->filename, info->lineno, info->name, fmtb);
      size = strlen(result) + 1;
      break;
  }
  free(fmtb);
  /* We can't use strdup here since %u and %z can insert NULL
   * characters into the stream. */
  *rtn = malloc(size*sizeof(char));
  memcpy(*rtn, result, size);
  free(result);
  return size - 1;
}

/* We can't use the normal str functions on the return value since
 * %u and %z can insert NULL characters into the stream. */
static unsigned int get_format(char **rtn, char *fmt,
                               const struct strobe_cb_info *info, unsigned int *idx)
{
  char *cp = fmt;
  unsigned int size;

  *rtn = strdup("");
  size = 1;
  while (*cp) {
    size_t cnt = strcspn(cp, "%");

    if (cnt > 0) {
      *rtn = realloc(*rtn, (size+cnt)*sizeof(char));
      memcpy(*rtn+size-1, cp, cnt);
      size += cnt;
      cp += cnt;
    } else {
      int ljust = 0, plus = 0, ld_zero = 0, width = -1, prec = -1;
      char *result;

      cp += 1;
      while ((*cp == '-') || (*cp == '+')) {
        if (*cp == '-') ljust = 1;
        else plus = 1;
        cp += 1;
      }
      if (*cp == '0') {
        ld_zero = 1;
        cp += 1;
      }
      if (isdigit((int)*cp)) width = strtoul(cp, &cp, 10);
      if (*cp == '.') {
        cp += 1;
        prec = strtoul(cp, &cp, 10);
      }
      cnt = get_format_char(&result, ljust, plus, ld_zero, width, prec, *cp,
                            info, idx);
      *rtn = realloc(*rtn, (size+cnt)*sizeof(char));
      memcpy(*rtn+size-1, result, cnt);
      free(result);
      size += cnt;
      if (*cp) cp += 1;
    }
  }
  *(*rtn+size-1) = '\0';
  return size - 1;
}

static unsigned int get_numeric(char **rtn, const struct strobe_cb_info *info,
                                vpiHandle item)
{
  int size, min;
  s_vpi_value val;

  val.format = info->default_format;
  vpi_get_value(item, &val);

  switch(info->default_format){
    case vpiDecStrVal:
      size = vpi_get_dec_size(item);
	/* -1 can be represented as a one bit signed value. This returns
	 * a size of 1 which is too small for the -1 string value so make
	 * the string width the minimum display width. */
      min = strlen(val.value.str);
      if (size < min) size = min;
      *rtn = malloc((size+1)*sizeof(char));
      sprintf(*rtn, "%*s", size, val.value.str);
      break;
    default:
      *rtn = strdup(val.value.str);
  }

  return strlen(*rtn);
}

/* In many places we can't use the normal str functions since %u and %z
 * can insert NULL characters into the stream. */
static char *get_display(unsigned int *rtnsz, const struct strobe_cb_info *info)
{
  char *result, *fmt, *rtn, *func_name;
  const char *cresult;
  s_vpi_value value;
  unsigned int idx, size, width;
  char buf[256];

  rtn = strdup("");
  size = 1;
  for  (idx = 0; idx < info->nitems; idx += 1) {
    vpiHandle item = info->items[idx];

    switch (vpi_get(vpiType, item)) {

      case vpiConstant:
      case vpiParameter:
        if (vpi_get(vpiConstType, item) == vpiStringConst) {
          value.format = vpiStringVal;
          vpi_get_value(item, &value);
          fmt = strdup(value.value.str);
          width = get_format(&result, fmt, info, &idx);
          free(fmt);
        } else if (vpi_get(vpiConstType, item) == vpiRealConst) {
          value.format = vpiRealVal;
          vpi_get_value(item, &value);
#if !defined(__GNUC__)
		  if (compatible_flag)
			  sprintf(buf, "%g", value.value.real);
		  else {
			  if (value.value.real == 0.0 || value.value.real == -0.0)
				  sprintf(buf, "%.05f", value.value.real);
			  else
				  sprintf(buf, "%#g", value.value.real);
		  }
#else
          sprintf(buf, compatible_flag ? "%g" : "%#g", value.value.real);
#endif
          result = strdup(buf);
          width = strlen(result);
        } else {
          width = get_numeric(&result, info, item);
        }
        rtn = realloc(rtn, (size+width)*sizeof(char));
        memcpy(rtn+size-1, result, width);
        free(result);
        break;

      case vpiNet:
      case vpiReg:
      case vpiBitVar:
      case vpiByteVar:
      case vpiShortIntVar:
      case vpiIntVar:
      case vpiLongIntVar:
      case vpiIntegerVar:
      case vpiMemoryWord:
      case vpiPartSelect:
        width = get_numeric(&result, info, item);
        rtn = realloc(rtn, (size+width)*sizeof(char));
        memcpy(rtn+size-1, result, width);
        free(result);
        break;

      /* It appears that this is not currently used! A time variable is
         passed as an integer and processed above. Hence this code has
         only been visually checked. */
      case vpiTimeVar:
        value.format = vpiDecStrVal;
        vpi_get_value(item, &value);
        get_time(buf, value.value.str, timeformat_info.prec,
                 vpi_get(vpiTimeUnit, info->scope));
        width = strlen(buf);
        if (width  < timeformat_info.width) width = timeformat_info.width;
        rtn = realloc(rtn, (size+width)*sizeof(char));
        sprintf(rtn+size-1, "%*s", width, buf);
        break;

      /* Realtime variables are also processed here. */
      case vpiRealVar:
        value.format = vpiRealVal;
        vpi_get_value(item, &value);
#if !defined(__GNUC__)
		if (compatible_flag)
			sprintf(buf, "%g", value.value.real);
		else {
			if (value.value.real == 0.0 || value.value.real == -0.0)
				sprintf(buf, "%.05f", value.value.real);
			else
				sprintf(buf, "%#g", value.value.real);
		}
#else
        sprintf(buf, compatible_flag ? "%g" : "%#g", value.value.real);
#endif
        width = strlen(buf);
        rtn = realloc(rtn, (size+width)*sizeof(char));
        memcpy(rtn+size-1, buf, width);
        break;

       /* Process string variables like string constants: interpret
	  the contained strings like format strings. */
      case vpiStringVar:
	value.format = vpiStringVal;
	vpi_get_value(item, &value);
	fmt = strdup(value.value.str);
	width = get_format(&result, fmt, info, &idx);
	free(fmt);
        rtn = realloc(rtn, (size+width)*sizeof(char));
        memcpy(rtn+size-1, result, width);
        free(result);
	break;

      case vpiSysFuncCall:
        func_name = vpi_get_str(vpiName, item);
        if (strcmp(func_name, "$time") == 0) {
          value.format = vpiDecStrVal;
          vpi_get_value(item, &value);
          width = strlen(value.value.str);
          if (width  < 20) width = 20;
          rtn = realloc(rtn, (size+width)*sizeof(char));
          sprintf(rtn+size-1, "%*s", width, value.value.str);

        } else if (strcmp(func_name, "$stime") == 0) {
          value.format = vpiDecStrVal;
          vpi_get_value(item, &value);
          width = strlen(value.value.str);
          if (width  < 10) width = 10;
          rtn = realloc(rtn, (size+width)*sizeof(char));
          sprintf(rtn+size-1, "%*s", width, value.value.str);

        } else if (strcmp(func_name, "$simtime") == 0) {
          value.format = vpiDecStrVal;
          vpi_get_value(item, &value);
          width = strlen(value.value.str);
          if (width  < 20) width = 20;
          rtn = realloc(rtn, (size+width)*sizeof(char));
          sprintf(rtn+size-1, "%*s", width, value.value.str);

        } else if (strcmp(func_name, "$realtime") == 0) {
          /* Use the local scope precision. */
          int use_prec = vpi_get(vpiTimeUnit, info->scope) -
                         vpi_get(vpiTimePrecision, info->scope);
          assert(use_prec >= 0);
          value.format = vpiRealVal;
          vpi_get_value(item, &value);
          sprintf(buf, "%.*f", use_prec, value.value.real);
          width = strlen(buf);
          rtn = realloc(rtn, (size+width)*sizeof(char));
          sprintf(rtn+size-1, "%*s", width, buf);

        } else {
          vpi_printf("WARNING: %s:%d: %s does not support %s as an argument!\n",
                     info->filename, info->lineno, info->name, func_name);
          strcpy(buf, "<?>");
          width = strlen(buf);
          rtn = realloc(rtn, (size+width)*sizeof(char));
          memcpy(rtn+size-1, buf, width);
        }
        break;

      default:
        vpi_printf("WARNING: %s:%d: unknown argument type (%s) given to %s!\n",
                   info->filename, info->lineno, vpi_get_str(vpiType, item),
                   info->name);
        cresult = "<?>";
        width = strlen(cresult);
        rtn = realloc(rtn, (size+width)*sizeof(char));
        memcpy(rtn+size-1, cresult, width);
        break;
    }
    size += width;
  }
  rtn[size-1] = '\0';
  *rtnsz = size - 1;
  return rtn;
}

#ifdef BR916_STOPGAP_FIX
static char br916_hint_issued = 0;
#endif

static int sys_check_args(vpiHandle callh, vpiHandle argv, const PLI_BYTE8*name,
                          int no_auto, int is_monitor)
{
      vpiHandle arg;
      int ret = 0;

        /* If there are no arguments, just return. */
      if (argv == 0) return ret;

      for (arg = vpi_scan(argv); arg; arg = vpi_scan(argv)) {
            if (no_auto && vpi_get(vpiAutomatic, arg)) {
                  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                             (int)vpi_get(vpiLineNo, callh));
                  vpi_printf("%s argument \"%s\" is an automatic variable.\n",
                             name, vpi_get_str(vpiName, arg));
                  ret = 1;
	    }

	    switch (vpi_get(vpiType, arg)) {
	      case vpiMemoryWord:
	      case vpiPartSelect:
		  if (is_monitor && vpi_get(vpiConstantSelect, arg) == 0) {
			vpi_printf("SORRY: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			vpi_printf("%s must have a constant %s select.\n",
			           name, vpi_get_str(vpiType, arg));
			ret = 1;
		  }
		  // fallthrough
	      case vpiConstant:
	      case vpiParameter:
	      case vpiNet:
	      case vpiReg:
	      case vpiIntegerVar:
	      case vpiBitVar:
	      case vpiByteVar:
	      case vpiShortIntVar:
	      case vpiIntVar:
	      case vpiLongIntVar:
	      case vpiTimeVar:
	      case vpiRealVar:
	      case vpiStringVar:
#ifdef BR916_STOPGAP_FIX
		    // no_auto implies either $strobe or $monitor
	          if (no_auto) {
		    switch (vpi_get(_vpiFromThr, arg)) {
		      case _vpiVThr:
		      case _vpiWord:
		      case _vpiString:
	                vpi_printf("SORRY: %s:%d: ",
				   vpi_get_str(vpiFile, callh),
	                           (int)vpi_get(vpiLineNo, callh));
	                vpi_printf("currently only simple signals or constant "
                                   "expressions may be passed to %s.\n", name);
			if (!br916_hint_issued) {
			      vpi_printf("NOTE: You can work around this by "
			                 "assigning the desired expression "
			                 "to an\n"
			                 "      intermediate net (using a "
			                 "continuous assignment) and passing "
			                 "that net\n"
			                 "      to %s.\n", name);
			      br916_hint_issued = 1;
			}
	                ret = 2;
		      default:
			break;
		    }
		  }
#endif
	      case vpiClassVar:
	      case vpiSysFuncCall:
		  break;

	      default:
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s does not support argument type (%s).\n", name,
		             vpi_get_str(vpiType, arg));
		  ret = 1;
		  break;
	    }
      }

      return ret;
}

/* Common compiletf routine. */
static PLI_INT32 sys_common_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name, int no_auto,
                                      int is_monitor)
{
      vpiHandle callh, argv;

      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);

      if (name[1] == 'f') {
	      /* Check that there is a fd/mcd and that it is numeric. */
	    if (argv == 0) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s requires at least a file descriptor/MCD.\n",
		             name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
		  return 0;
	    }

	    if (! is_numeric_obj(vpi_scan(argv))) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's file descriptor/MCD must be numeric.\n",
		             name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }
      }

      int rtn = sys_check_args(callh, argv, name, no_auto, is_monitor);
      if (rtn) {
	    if (rtn==1) vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }
      return 0;
}

/* Check the $display, $write, $fdisplay and $fwrite based tasks. */
static PLI_INT32 sys_display_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
	/* These tasks can have automatic variables and are not monitor. */
      return sys_common_compiletf(name, 0, 0);
}

/* This implements the $sformatf, $display/$fdisplay
 * and the $write/$fwrite based tasks. */
static PLI_INT32 sys_display_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, scope;
      struct strobe_cb_info info;
      char* result;
      unsigned int size;
      PLI_UINT32 fd_mcd;
      s_vpi_value val;

      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);

	/* Get the file/MC descriptor and verify it is valid. */
      if (name[1] == 'f') {
	    vpiHandle fd = vpi_scan(argv);
	    if (get_fd_mcd_from_arg(&fd_mcd, fd, callh, name)) {
		  vpi_free_object(argv);
		  return 0;
	    }
      } else if (strncmp(name, "$sformatf", 9) == 0) {
	      /* return as a string */
	    fd_mcd = 0;
      } else {
	      /* stdout */
	    fd_mcd = 1;
      }

      scope = vpi_handle(vpiScope, callh);
      assert(scope);
	/* We could use vpi_get_str(vpiName, callh) to get the task name,
	 * but name is already defined. */
      info.name = name;
      info.filename = strdup(vpi_get_str(vpiFile, callh));
      info.lineno = (int)vpi_get(vpiLineNo, callh);
      info.default_format = get_default_format(name);
      info.scope = scope;
      array_from_iterator(&info, argv);

	/* Because %u and %z may put embedded NULL characters into the
	 * returned string strlen() may not match the real size! */
      result = get_display(&size, &info);

      if (fd_mcd > 0) {
	     my_mcd_rawwrite(fd_mcd, result, size);
	     if ((strncmp(name,"$display",8) == 0) ||
	         (strncmp(name,"$fdisplay",9) == 0)) my_mcd_rawwrite(fd_mcd, "\n", 1);
      } else {
	       /* Return as a string ($sformatf) */
	     val.format = vpiStringVal;
	     val.value.str = result;
	     vpi_put_value(callh, &val, 0, vpiNoDelay);
      }

      free(result);
      free(info.filename);
      free(info.items);
      return 0;
}

/*
 * The strobe implementation takes the parameter handles that are
 * passed to the calltf and puts them in to an array for safe
 * keeping. That array (and other bookkeeping) is passed, via the
 * struct_cb_info object, to the REadOnlySych function strobe_cb,
 * where it is used to perform the actual formatting and printing.
 */
static PLI_INT32 strobe_cb(p_cb_data cb)
{
      struct strobe_cb_info*info = (struct strobe_cb_info*)cb->user_data;

	/* We really need to cancel any $fstrobe() calls for a file when it
	 * is closed, but for now we will just skip processing the result.
	 * Which has the same basic effect. */
      if (is_valid_fd_mcd(info->fd_mcd)) {
	    char* result;
	    unsigned int size;
	      /* Because %u and %z may put embedded NULL characters into the
	       * returned string strlen() may not match the real size! */
	    result = get_display(&size, info);
	    my_mcd_rawwrite(info->fd_mcd, result, size);
	    my_mcd_rawwrite(info->fd_mcd, "\n", 1);
	    free(result);
      }

      free(info->filename);
      free(info->items);
      free(info);
      return 0;
}

/* Check both the $strobe and $fstrobe based tasks. */
static PLI_INT32 sys_strobe_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
	/* These tasks can not have automatic variables and are not monitor. */
      return sys_common_compiletf(name, 1, 0);
}

/* This implements both the $strobe and $fstrobe based tasks. */
static PLI_INT32 sys_strobe_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh, argv, scope;
      struct t_cb_data cb;
      struct t_vpi_time timerec;
      struct strobe_cb_info*info;
      PLI_UINT32 fd_mcd;

      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);

	/* Get the file/MC descriptor and verify it is valid. */
      if (name[1] == 'f') {
	    vpiHandle fd = vpi_scan(argv);
	    if (get_fd_mcd_from_arg(&fd_mcd, fd, callh, name)) {
		  vpi_free_object(argv);
                  return 0;
	    }

      } else {
	    fd_mcd = 1;
      }

      scope = vpi_handle(vpiScope, callh);
      assert(scope);

      info = calloc(1, sizeof(struct strobe_cb_info));
      info->fd_mcd = fd_mcd;
	/* We could use vpi_get_str(vpiName, callh) to get the task name,
	 * but name is already defined. */
      info->name = name;
      info->filename = strdup(vpi_get_str(vpiFile, callh));
      info->lineno = (int)vpi_get(vpiLineNo, callh);
      info->default_format = get_default_format(name);
      info->scope= scope;
      array_from_iterator(info, argv);

      timerec.type = vpiSimTime;
      timerec.low = 0;
      timerec.high = 0;

      cb.reason = cbReadOnlySynch;
      cb.cb_rtn = strobe_cb;
      cb.time = &timerec;
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

static struct strobe_cb_info monitor_info = { 0, 0, 0, 0, 0, 0, 0, 0 };
static vpiHandle *monitor_callbacks = 0;
static int monitor_scheduled = 0;
static int monitor_enabled = 1;

static PLI_INT32 monitor_cb_2(p_cb_data cb)
{
      char* result;
      unsigned int size;

      (void)cb; /* Parameter is not used. */

	/* Because %u and %z may put embedded NULL characters into the
	 * returned string strlen() may not match the real size! */
      result = get_display(&size, &monitor_info);
      my_mcd_rawwrite(monitor_info.fd_mcd, result, size);
      my_mcd_rawwrite(monitor_info.fd_mcd, "\n", 1);
      monitor_scheduled = 0;
      free(result);
      return 0;
}

/*
 * The monitor_cb_1 callback is called when an event occurs somewhere
 * in the simulation. All this function does is schedule the actual
 * display to occur in a ReadOnlySync callback. The monitor_scheduled
 * flag is used to allow only one monitor strobe to be scheduled.
 */
static PLI_INT32 monitor_cb_1(p_cb_data cause)
{
      struct t_cb_data cb;
      struct t_vpi_time timerec;

      (void)cause; /* Parameter is not used. */

      if (monitor_enabled == 0) return 0;
      if (monitor_scheduled) return 0;

	/* This this action caused the first trigger, then schedule
	   the monitor to happen at the end of the time slice and mark
	   it as scheduled. */
      monitor_scheduled += 1;
      timerec.type = vpiSimTime;
      timerec.low = 0;
      timerec.high = 0;

      cb.reason = cbReadOnlySynch;
      cb.cb_rtn = monitor_cb_2;
      cb.time = &timerec;
      cb.obj = 0;
      cb.value = 0;
      vpi_register_cb(&cb);

      return 0;
}

static PLI_INT32 sys_monitor_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      int rtn = sys_check_args(callh, argv, name, 1, 1);
      if (rtn) {
	    if (rtn == 1) vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }
      return 0;
}

static PLI_INT32 sys_monitor_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh, argv, scope;
      unsigned idx;
      struct t_cb_data cb;
      struct t_vpi_time timerec;

      (void)name; /* Parameter is not used. */

      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);

	/* If there was a previous $monitor, then remove the callbacks
	   related to it. */
      if (monitor_callbacks) {
	    for (idx = 0 ;  idx < monitor_info.nitems ;  idx += 1)
		  if (monitor_callbacks[idx])
			vpi_remove_cb(monitor_callbacks[idx]);

	    free(monitor_callbacks);
	    monitor_callbacks = 0;

	    free(monitor_info.filename);
	    free(monitor_info.items);
	    monitor_info.items = 0;
	    monitor_info.nitems = 0;
	    monitor_info.name = 0;
      }

      scope = vpi_handle(vpiScope, callh);
      assert(scope);
	/* Make an array of handles from the argument list. */
      array_from_iterator(&monitor_info, argv);
      monitor_info.name = name;
      monitor_info.filename = strdup(vpi_get_str(vpiFile, callh));
      monitor_info.lineno = (int)vpi_get(vpiLineNo, callh);
      monitor_info.default_format = get_default_format(name);
      monitor_info.scope = scope;
      monitor_info.fd_mcd = 1;

	/* Attach callbacks to all the parameters that might change. */
      monitor_callbacks = calloc(monitor_info.nitems, sizeof(vpiHandle));

      timerec.type = vpiSuppressTime;
      cb.reason = cbValueChange;
      cb.cb_rtn = monitor_cb_1;
      cb.time = &timerec;
      cb.value = NULL;
      for (idx = 0 ;  idx < monitor_info.nitems ;  idx += 1) {

	    switch (vpi_get(vpiType, monitor_info.items[idx])) {
		case vpiMemoryWord:
		  /*
		   * We only support constant selections. Make this
		   * better when we add a real compiletf routine.
		   */
		  assert(vpi_get(vpiConstantSelect, monitor_info.items[idx]));
		case vpiNet:
		case vpiReg:
		case vpiIntegerVar:
		case vpiBitVar:
		case vpiByteVar:
		case vpiShortIntVar:
		case vpiIntVar:
		case vpiLongIntVar:
		case vpiRealVar:
		case vpiPartSelect:
		    /* Monitoring reg and net values involves setting
		       a callback for value changes. Pass the storage
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

static PLI_INT32 sys_monitoron_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name; /* Parameter is not used. */
      monitor_enabled = 1;
      monitor_cb_1(0);
      return 0;
}

static PLI_INT32 sys_monitoroff_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name; /* Parameter is not used. */
      monitor_enabled = 0;
      return 0;
}

static PLI_INT32 sys_swrite_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
  vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
  vpiHandle argv = vpi_iterate(vpiArgument, callh);
  vpiHandle arg;
  PLI_INT32 type;

  /* Check that there are arguments. */
  if (argv == 0) {
    vpi_printf("ERROR:%s:%d: ", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
    vpi_printf("%s requires at least one argument.\n", name);
    vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
    return 0;
  }

  /* The first argument must be a register or a SV string. */
  arg = vpi_scan(argv);  /* This should never be zero. */
  type = vpi_get(vpiType, arg);
  if (type != vpiReg && type != vpiStringVar) {
    vpi_printf("ERROR:%s:%d: ", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
    vpi_printf("%s's first argument must be a register or SV string.\n", name);
    vpi_control(vpiFinish, 1);
    return 0;
  }

  int rtn = sys_check_args(callh, argv, name, 0, 0);
  if (rtn) {
    if (rtn == 1) vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
  }
  return 0;
}

static PLI_INT32 sys_swrite_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
  vpiHandle callh, argv, reg, scope;
  struct strobe_cb_info info;
  s_vpi_value val;
  unsigned int size;

  callh = vpi_handle(vpiSysTfCall, 0);
  argv = vpi_iterate(vpiArgument, callh);
  reg = vpi_scan(argv);

  scope = vpi_handle(vpiScope, callh);
  assert(scope);
  /* We could use vpi_get_str(vpiName, callh) to get the task name, but
   * name is already defined. */
  info.name = name;
  info.filename = strdup(vpi_get_str(vpiFile, callh));
  info.lineno = (int)vpi_get(vpiLineNo, callh);
  info.default_format = get_default_format(name);
  info.scope = scope;
  array_from_iterator(&info, argv);

  /* Because %u and %z may put embedded NULL characters into the returned
   * string strlen() may not match the real size! */
  val.value.str = get_display(&size, &info);
  val.format = vpiStringVal;
  vpi_put_value(reg, &val, 0, vpiNoDelay);
  if (size != strlen(val.value.str)) {
    vpi_printf("WARNING: %s:%d: %s returned a value with an embedded NULL "
               "(see %%u/%%z).\n", info.filename, info.lineno, name);
  }

  free(val.value.str);
  free(info.filename);
  free(info.items);
  return 0;
}

static PLI_INT32 sys_sformat_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
  vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
  vpiHandle argv = vpi_iterate(vpiArgument, callh);
  vpiHandle arg;
  PLI_INT32 type;

  /* Check that there are arguments. */
  if (argv == 0) {
    vpi_printf("ERROR:%s:%d: ", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
    vpi_printf("%s requires at least two arguments.\n", name);
    vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
    return 0;
  }

  /* The first argument must be a register or a SV string. */
  arg = vpi_scan(argv);  /* This should never be zero. */
  type = vpi_get(vpiType, arg);
  if (type != vpiReg && type != vpiStringVar) {
    vpi_printf("ERROR:%s:%d: ", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
    vpi_printf("%s's first argument must be a register or SV string.\n", name);
    vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
    return 0;
  }

  /* The second argument must be a string, a register or a SV string. */
  arg = vpi_scan(argv);
  if (arg == 0) {
    vpi_printf("ERROR:%s:%d: ", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
    vpi_printf("%s requires at least two arguments.\n", name);
    vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
    return 0;
  }
  type = vpi_get(vpiType, arg);
  if (((type != vpiConstant && type != vpiParameter) ||
      vpi_get(vpiConstType, arg) != vpiStringConst) &&
      type != vpiReg && type != vpiStringVar) {
    vpi_printf("ERROR:%s:%d: ", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
    vpi_printf("%s's second argument must be a string or a register.\n", name);
    vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
    return 0;
  }

  int rtn = sys_check_args(callh, argv, name, 0, 0);
  if (rtn) {
    if (rtn == 1) vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
  }
  return 0;
}

static PLI_INT32 sys_sformat_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
  vpiHandle callh, argv, reg, scope;
  struct strobe_cb_info info;
  s_vpi_value val;
  char *result, *fmt;
  unsigned int idx, size;

  callh = vpi_handle(vpiSysTfCall, 0);
  argv = vpi_iterate(vpiArgument, callh);
  reg = vpi_scan(argv);
  val.format = vpiStringVal;
  vpi_get_value(vpi_scan(argv), &val);
  fmt = strdup(val.value.str);

  scope = vpi_handle(vpiScope, callh);
  assert(scope);
  /* We could use vpi_get_str(vpiName, callh) to get the task name, but
   * name is already defined. */
  info.name = name;
  info.filename = strdup(vpi_get_str(vpiFile, callh));
  info.lineno = (int)vpi_get(vpiLineNo, callh);
  info.default_format = get_default_format(name);
  info.scope = scope;
  array_from_iterator(&info, argv);
  idx = -1;
  size = get_format(&result, fmt, &info, &idx);
  free(fmt);

  if (idx+1< info.nitems) {
    vpi_printf("WARNING: %s:%d: %s has %d extra argument(s).\n",
               info.filename, info.lineno,  name,
               info.nitems-idx-1);
  }

  val.value.str = result;
  val.format = vpiStringVal;
  vpi_put_value(reg, &val, 0, vpiNoDelay);
  if (size != strlen(val.value.str)) {
    vpi_printf("WARNING: %s:%d: %s returned a value with an embedded NULL "
               "(see %%u/%%z).\n", info.filename, info.lineno, name);
  }

  free(val.value.str);
  free(info.filename);
  free(info.items);
  return 0;
}

static PLI_INT32 sys_sformatf_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
  vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
  vpiHandle argv = vpi_iterate(vpiArgument, callh);
  vpiHandle arg;
  PLI_INT32 type;

  /* Check that there are arguments. */
  if (argv == 0) {
    vpi_printf("ERROR:%s:%d: ", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
    vpi_printf("%s requires at least one argument.\n", name);
    vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
    return 0;
  }

  /* The first argument must be a string, a register or a SV string. */
  arg = vpi_scan(argv);
  if (arg == 0) {
    vpi_printf("ERROR:%s:%d: ", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
    vpi_printf("%s requires at least one argument.\n", name);
    vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
    return 0;
  }
  type = vpi_get(vpiType, arg);
  if (((type != vpiConstant && type != vpiParameter) ||
      vpi_get(vpiConstType, arg) != vpiStringConst) &&
      type != vpiReg && type != vpiStringVar) {
    vpi_printf("ERROR:%s:%d: ", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
    vpi_printf("%s's first argument must be a string or a register.\n", name);
    vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
    return 0;
  }

  int rtn = sys_check_args(callh, argv, name, 0, 0);
  if (rtn) {
    if (rtn == 1) vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
  }
  return 0;
}

static PLI_INT32 sys_end_of_compile(p_cb_data cb_data)
{
      (void)cb_data; /* Parameter is not used. */
	/* The default timeformat prints times in unit of simulation
	   precision. */
      free(timeformat_info.suff);
      timeformat_info.suff  = strdup("");
      timeformat_info.units = vpi_get(vpiTimePrecision, 0);
      timeformat_info.prec  = 0;
      timeformat_info.width = 20;
      return 0;
}

static PLI_INT32 sys_timeformat_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh   = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv  = vpi_iterate(vpiArgument, callh);

      if (argv) {
	    vpiHandle arg;

	      /* Check that the unit argument is numeric. */
	    if (! is_numeric_obj(vpi_scan(argv))) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's units argument must be numeric.\n", name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }

	      /* Check that the precision argument is given and is numeric. */
	    arg = vpi_scan(argv);
	    if (! arg) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s requires zero or four arguments.\n", name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
		  return 0;
	    }

	    if (! is_numeric_obj(arg)) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's precision argument must be numeric.\n",
		             name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }

	      /* Check that the suffix argument is given and is a string. */
	    arg = vpi_scan(argv);
	    if (! arg) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s requires zero or four arguments.\n", name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
		  return 0;
	    }

	    if (! is_string_obj(arg)) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's suffix argument must be a string.\n", name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }

	      /* Check that the min. width argument is given and is numeric. */
	    arg = vpi_scan(argv);
	    if (! arg) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s requires zero or four arguments.\n", name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
		  return 0;
	    }

	    if (! is_numeric_obj(arg)) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's minimum width argument must be numeric.\n",
		             name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }

	      /* Make sure there are no extra arguments. */
	    check_for_extra_args(argv, callh, name, "four arguments", 0);
      }

      return 0;
}

static PLI_INT32 sys_timeformat_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      s_vpi_value value;
      vpiHandle sys   = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv  = vpi_iterate(vpiArgument, sys);

      (void)name; /* Parameter is not used. */

      if (argv) {
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
            free(timeformat_info.suff);
            timeformat_info.suff = strdup(value.value.str);

            value.format = vpiIntVal;
            vpi_get_value(wid, &value);
            timeformat_info.width = value.value.integer;
      } else {
            /* If no arguments are given then use the default values. */
            sys_end_of_compile(NULL);
      }

      return 0;
}

static const char *pts_convert(int value)
{
      const char *string;
      switch (value) {
            case   2: string = "100s";  break;
            case   1: string = "10s";   break;
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
            default: string = "invalid"; assert(0);
      }
      return string;
}

static PLI_INT32 sys_printtimescale_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh   = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv  = vpi_iterate(vpiArgument, callh);

      if (argv) {
	    vpiHandle arg = vpi_scan(argv);
	    switch (vpi_get(vpiType, arg)) {
		case vpiFunction:
		case vpiGenScope:
		case vpiIntegerVar:
		case vpiBitVar:
		case vpiByteVar:
		case vpiShortIntVar:
		case vpiIntVar:
		case vpiLongIntVar:
		case vpiMemory:
		case vpiMemoryWord:
		case vpiModule:
		case vpiNamedBegin:
		case vpiNamedEvent:
		case vpiNamedFork:
		case vpiNet:
		case vpiNetArray:
//		case vpiNetBit: // Unused and unavailable in Icarus
		case vpiParameter:
		case vpiPartSelect:
		case vpiRealVar:
		case vpiReg:
//		case vpiRegBit: // Unused and unavailable in Icarus
		case vpiTask:
		case vpiTimeVar: // Unused in Icarus
		  break;

		default:
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's argument must have a module, given a %s.\n",
		             name, vpi_get_str(vpiType, arg));
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }

	      /* Make sure there are no extra arguments. */
	    check_for_extra_args(argv, callh, name, "one argument", 1);
      }

      return 0;
}

static PLI_INT32 sys_printtimescale_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh   = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv  = vpi_iterate(vpiArgument, callh);
      vpiHandle item, scope;

      (void)name; /* Parameter is not used. */

      if (!argv) {
            item = sys_func_module(callh);
      } else {
            item = vpi_scan(argv);
            vpi_free_object(argv);
      }
      scope = sys_func_module(item);

      vpi_printf("Time scale of (%s) is ", vpi_get_str(vpiFullName, item));
      vpi_printf("%s / ", pts_convert(vpi_get(vpiTimeUnit, scope)));
      vpi_printf("%s\n", pts_convert(vpi_get(vpiTimePrecision, scope)));

      return 0;
}

static PLI_INT32 sys_fatal_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      if (argv) {
            vpiHandle arg = vpi_scan(argv);

            /* Check that finish_number (1st argument) is numeric */
            if (! is_numeric_obj(arg)) {
                  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
                             (int)vpi_get(vpiLineNo, callh));
                  vpi_printf("%s's finish number must be numeric.\n", name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
            }

	    int rtn = sys_check_args(callh, argv, name, 0, 0);
	    if (rtn) {
		  if (rtn == 1) vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }
      }

      return 0;
}

static PLI_INT32 sys_severity_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle scope;
      struct strobe_cb_info info;
      struct t_vpi_time now;
      PLI_UINT64 now64;
      char *sstr, *t, *dstr;
      unsigned int size, location=0;
      s_vpi_value finish_number;

      /* Set the default finish number for $fatal. */
      finish_number.value.integer = 1;

      /* Check that the finish number is in range. */
      if (strncmp(name,"$fatal", 6) == 0 && argv) {
            vpiHandle arg = vpi_scan(argv);
            finish_number.format = vpiIntVal;
            vpi_get_value(arg, &finish_number);
            if ((finish_number.value.integer < 0) ||
		(finish_number.value.integer > 2)) {
                  vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
                             (int)vpi_get(vpiLineNo, callh));
                  vpi_printf("$fatal called with finish_number of %d, "
			     "but it must be 0, 1, or 2.\n",
			     (int)finish_number.value.integer);
		  finish_number.value.integer = 1;
            }
      }

      /* convert name to upper and drop $ to get severity string */
      sstr = strdup(name) + 1;
      for (t=sstr; *t; t+=1) *t = toupper((int)*t);

      scope = vpi_handle(vpiScope, callh);
      assert(scope);
      info.name = name;
      info.filename = strdup(vpi_get_str(vpiFile, callh));
      info.lineno = (int)vpi_get(vpiLineNo, callh);
      info.default_format = vpiDecStrVal;
      info.scope = scope;
      array_from_iterator(&info, argv);

      vpi_printf("%s: %s:%d: ", sstr, info.filename, info.lineno);

      dstr = get_display(&size, &info);
      while (location < size) {
	    if (dstr[location] == '\0') {
		  my_mcd_printf(1, "%c", '\0');
		  location += 1;
	    } else {
		  my_mcd_printf(1, "%s", &dstr[location]);
		  location += strlen(&dstr[location]);
	    }
      }

      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      now64 = timerec_to_time64(&now);

      vpi_printf("\n%*s  Time: %" PLI_UINT64_FMT "  Scope: %s\n",
                 (int)strlen(sstr), " ", now64,
                 vpi_get_str(vpiFullName, scope));

      free(--sstr);  /* Get the $ back. */
      free(info.filename);
      free(info.items);
      free(dstr);

      if (strncmp(name,"$fatal",6) == 0) {
	      /* Set the exit code from vvp as an error code. */
	    vpip_set_return_value(1);
	      /* Now tell the simulator to finish. */
            vpi_control(vpiFinish, finish_number.value.integer);
      }

      return 0;
}


static PLI_INT32 sys_end_of_simulation(p_cb_data cb_data)
{
      (void)cb_data; /* Parameter is not used. */
      free(monitor_callbacks);
      monitor_callbacks = 0;
      free(monitor_info.filename);
      free(monitor_info.items);
      monitor_info.items = 0;
      monitor_info.nitems = 0;
      monitor_info.name = 0;

      free(timeformat_info.suff);
      timeformat_info.suff = 0;
      return 0;
}

void sys_display_register(void)
{
      s_cb_data cb_data;
      s_vpi_systf_data tf_data;
      vpiHandle res;

      check_command_line_args();

      /*============================== display */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$display";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$display";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$displayh";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$displayh";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$displayo";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$displayo";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$displayb";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$displayb";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== write */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$write";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$write";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$writeh";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$writeh";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$writeo";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$writeo";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$writeb";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$writeb";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== strobe */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$strobe";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = sys_strobe_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$strobe";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$strobeh";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = sys_strobe_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$strobeh";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$strobeo";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = sys_strobe_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$strobeo";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$strobeb";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = sys_strobe_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$strobeb";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fstrobe */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fstrobe";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = sys_strobe_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fstrobe";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fstrobeh";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = sys_strobe_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fstrobeh";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fstrobeo";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = sys_strobe_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fstrobeo";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fstrobeb";
      tf_data.calltf    = sys_strobe_calltf;
      tf_data.compiletf = sys_strobe_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fstrobeb";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== monitor */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitor";
      tf_data.calltf    = sys_monitor_calltf;
      tf_data.compiletf = sys_monitor_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitor";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitorh";
      tf_data.calltf    = sys_monitor_calltf;
      tf_data.compiletf = sys_monitor_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitorh";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitoro";
      tf_data.calltf    = sys_monitor_calltf;
      tf_data.compiletf = sys_monitor_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitoro";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitorb";
      tf_data.calltf    = sys_monitor_calltf;
      tf_data.compiletf = sys_monitor_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitorb";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitoron";
      tf_data.calltf    = sys_monitoron_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitoron";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$monitoroff";
      tf_data.calltf    = sys_monitoroff_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$monitoroff";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fdisplay */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fdisplay";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fdisplay";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fdisplayh";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fdisplayh";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fdisplayo";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fdisplayo";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fdisplayb";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fdisplayb";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fwrite */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fwrite";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fwrite";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fwriteh";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fwriteh";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fwriteo";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fwriteo";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fwriteb";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fwriteb";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== swrite */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$swrite";
      tf_data.calltf    = sys_swrite_calltf;
      tf_data.compiletf = sys_swrite_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$swrite";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$swriteh";
      tf_data.calltf    = sys_swrite_calltf;
      tf_data.compiletf = sys_swrite_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$swriteh";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$swriteo";
      tf_data.calltf    = sys_swrite_calltf;
      tf_data.compiletf = sys_swrite_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$swriteo";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$swriteb";
      tf_data.calltf    = sys_swrite_calltf;
      tf_data.compiletf = sys_swrite_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$swriteb";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$sformat";
      tf_data.calltf    = sys_sformat_calltf;
      tf_data.compiletf = sys_sformat_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$sformat";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiStringFunc;
      tf_data.tfname    = "$sformatf";
      tf_data.calltf    = sys_display_calltf;
      tf_data.compiletf = sys_sformatf_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$sformatf";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================ timeformat */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$timeformat";
      tf_data.calltf    = sys_timeformat_calltf;
      tf_data.compiletf = sys_timeformat_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$timeformat";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$printtimescale";
      tf_data.calltf    = sys_printtimescale_calltf;
      tf_data.compiletf = sys_printtimescale_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$printtimescale";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================ severity tasks */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fatal";
      tf_data.calltf    = sys_severity_calltf;
      tf_data.compiletf = sys_fatal_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fatal";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$error";
      tf_data.calltf    = sys_severity_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$error";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$warning";
      tf_data.calltf    = sys_severity_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$warning";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$info";
      tf_data.calltf    = sys_severity_calltf;
      tf_data.compiletf = sys_display_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$info";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      cb_data.reason = cbEndOfCompile;
      cb_data.time = 0;
      cb_data.cb_rtn = sys_end_of_compile;
      cb_data.user_data = "system";
      vpi_register_cb(&cb_data);

      cb_data.reason = cbEndOfSimulation;
      cb_data.cb_rtn = sys_end_of_simulation;
      cb_data.user_data = "system";
      vpi_register_cb(&cb_data);
}
