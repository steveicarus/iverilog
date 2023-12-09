/*
 * Copyright (c) 2003-2023 Stephen Williams (steve@icarus.com)
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
# include  <ctype.h>
# include  <errno.h>
# include  <string.h>
# include  <stdio.h>
# include  <stdlib.h>
# include  "ivl_alloc.h"


/*
 * Implement the $fopen system function.
 */
static PLI_INT32 sys_fopen_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;
      assert(callh != 0);
      argv = vpi_iterate(vpiArgument, callh);

	/* Check that there is a file name argument and that it is a string. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a string file name argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      if (! is_string_obj(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's file name argument must be a string.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* The type argument is optional. */
      arg = vpi_scan(argv);
      if (arg == 0) return 0;

	/* When provided, the type argument must be a string. */
      if (! is_string_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's type argument must be a string.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "two string arguments", 1);

      return 0;
}

static PLI_INT32 sys_fopen_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      s_vpi_value val;
      int fail = 0;
      char *mode_string = 0;
      vpiHandle fileh = vpi_scan(argv);
      char *fname;
      vpiHandle mode = vpi_scan(argv);
      errno = 0;

	/* Get the mode handle if it exists. */
      if (mode) {
	    char *esc_md;
	    val.format = vpiStringVal;
	    vpi_get_value(mode, &val);
	      /* Verify that we have a string and that it is not NULL. */
	    if (val.format != vpiStringVal || !*(val.value.str)) {
		  vpi_printf("WARNING: %s:%d: ",
		             vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's mode argument is not a valid string.\n",
		             name);
		  fail = 1;
	    }

	      /* Make sure the mode string is correct. */
	    if (strlen(val.value.str) > 3) {
		  vpi_printf("WARNING: %s:%d: ",
		             vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  esc_md = as_escaped(val.value.str);
		  vpi_printf("%s's mode argument (%s) is too long.\n",
		             name, esc_md);
		  free(esc_md);
		  fail = 1;
	    } else {
		  unsigned bin = 0, plus = 0, idx;
		  switch (val.value.str[0]) {
		      case 'r':
		      case 'w':
		      case 'a':
			for (idx = 1; idx < 3 ; idx++) {
			      if (val.value.str[idx] == '\0') break;
			      switch (val.value.str[idx]) {
				    case 'b':
				      if (bin) fail = 1;
				      bin = 1;
				      break;
				    case '+':
				      if (plus) fail = 1;
				      plus = 1;
				      break;
				    default:
				      fail = 1;
				      break;
			      }
			}
			if (! fail) break;
			// fallthrough
		      default:
			vpi_printf("WARNING: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			esc_md = as_escaped(val.value.str);
			vpi_printf("%s's mode argument (%s) is invalid.\n",
			name, esc_md);
			free(esc_md);
			fail = 1;
			break;
		  }
	    }

	    mode_string = strdup(val.value.str);

	    vpi_free_object(argv);
      }

      fname = get_filename(callh, name, fileh);

	/* If either the mode or file name are not valid just return. */
      if (fail || fname == 0) {
	    free(fname);
	    if (mode) free(mode_string);
	    return 0;
      }

      val.format = vpiIntVal;
      if (mode) {
	    val.value.integer = vpi_fopen(fname, mode_string);
	    free(mode_string);
      } else
	    val.value.integer = vpi_mcd_open(fname);

      vpi_put_value(callh, &val, 0, vpiNoDelay);
      free(fname);

      return 0;
}

/*
 * Implement the $fopenr(), $fopenw() and $fopena() system functions
 * from Chris Spear's File I/O for Verilog.
 */

static PLI_INT32 sys_fopenrwa_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      s_vpi_value val;
      char *fname;
      const char *mode;
      errno = 0;

	/* Get the mode. */
      mode = name + strlen(name) - 1;

	/* Get the file name. */
      fname = get_filename(callh, name, vpi_scan(argv));
      vpi_free_object(argv);
      if (fname == 0) return 0;

	/* Open the file and return the result. */
      val.format = vpiIntVal;
      val.value.integer = vpi_fopen(fname, mode);
      vpi_put_value(callh, &val, 0, vpiNoDelay);
      free(fname);

      return 0;
}

/*
 * Implement $fclose system function
 */
static PLI_INT32 sys_fclose_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle fd = vpi_scan(argv);
      PLI_UINT32 fd_mcd;

      vpi_free_object(argv);

	/* Get the file/MC descriptor and verify that it is valid. */
      if (get_fd_mcd_from_arg(&fd_mcd, fd, callh, name)) return 0;

	/* We need to cancel any active $fstrobe()'s for this FD/MCD.
	 * For now we check in the strobe callback and skip the output
	 * generation when needed. */
      fd_mcd = vpi_mcd_close(fd_mcd);

      if (fd_mcd) {
	    char *fd_mcd_name;
	    switch(fd_mcd) {
		case 0x00000001: // MCD STDOUT
		case 0x80000001: // FD STDOUT
		  fd_mcd_name = "STDOUT ";
		  break;
		case 0x80000000: // FD STDIN
		  fd_mcd_name = "STDIN ";
		  break;
		case 0x80000002: // FD STDERR
		  fd_mcd_name = "STDERR ";
		  break;
		default:
		  fd_mcd_name = "";
	    }
            vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
                       (int)vpi_get(vpiLineNo, callh));
            vpi_printf("could not close %s %s(0x%x) in %s().\n",
	               IS_MCD(fd_mcd) ? "MCD" : "file descriptor",
                       fd_mcd_name, (unsigned int)fd_mcd, name);
      }

      return 0;
}

/*
 * Implement $fflush system function
 */
static PLI_INT32 sys_fflush_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle fd;
      PLI_UINT32 fd_mcd;

	/* If we have no argument then flush all the streams. */
      if (argv == 0) {
	    fflush(NULL);
	    return 0;
      }

	/* Get the file/MC descriptor and verify that it is valid. */
      fd = vpi_scan(argv);
      vpi_free_object(argv);
      if (get_fd_mcd_from_arg(&fd_mcd, fd, callh, name)) return 0;

      if (IS_MCD(fd_mcd)) {
	    vpi_mcd_flush(fd_mcd);
      } else {
	    fflush(vpi_get_file(fd_mcd));
      }

      return 0;
}

static PLI_INT32 sys_fputc_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      FILE *fp;
      unsigned char chr;
      errno = 0;

	/* Get the character. */
      arg = vpi_scan(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      chr = val.value.integer;


	/* Get the file/MC descriptor. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

	/* Put the character and return the result. */
      fp = vpi_get_file(fd_mcd);
      val.format = vpiIntVal;
      if (!fp) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n",
	               (unsigned int)fd_mcd, name);
	    errno = EBADF;
	    val.value.integer = EOF;
      } else {
	    val.value.integer = fputc(chr, fp);
	    if (val.value.integer != EOF) val.value.integer = 0;
      }
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 sys_fgets_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

	/*
	 * Check that there are two arguments and that the first is a
	 * variable and that the second is numeric.
	 */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      if (! is_variable(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument must be a variable.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a second (numeric) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      if (! is_numeric_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's second argument must be numeric.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "two arguments", 0);

      return 0;
}

static void return_zero(vpiHandle callh)
{
      s_vpi_value val;
      val.format = vpiIntVal;
      val.value.integer = 0;
      assert(callh);
      vpi_put_value(callh, &val, 0, vpiNoDelay);
}

static PLI_INT32 sys_fgets_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle varh;
      vpiHandle arg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      FILE *fp;
      PLI_INT32 var_size;
      char*text;
      errno = 0;

	/* Get the variable handle. */
      varh = vpi_scan(argv);

	/* Get the file/MCD descriptor. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

	/* Return zero if this is not a valid fd. */
      fp = vpi_get_file(fd_mcd);
      if (!fp) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n",
	               (unsigned int)fd_mcd, name);
	    errno = EBADF;
	    val.format = vpiIntVal;
	    val.value.integer = 0;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    return 0;
      }

      if (vpi_get(vpiType, varh) == vpiStringVar) {
	    PLI_INT32 base_size = 1024;
	    size_t txt_len;
	    var_size = 0;
	    text = 0;
	    do {
		  var_size += base_size;
		  text = realloc(text, var_size+1);
		    /* Read in the bytes. Return 0 if there was an error.
		     * Assume an error can only occur when reading the first group of bytes. */
		  if ((fgets(text+var_size-base_size, base_size+1, fp) == 0) && (var_size == base_size)) {
			return_zero(callh);
			free(text);
			return 0;
		  }
		  txt_len = strlen(text);
	    } while (((PLI_INT32) txt_len == var_size) && (text[txt_len-1] != '\n'));
      } else {
	      /* Get the register size in bytes and allocate the buffer. */
	    var_size = vpi_get(vpiSize, varh) / 8;
	    text = malloc(var_size + 1);

	      /* Read in the bytes. Return 0 if there was an error. */
	    if (fgets(text, var_size+1, fp) == 0) {
		  return_zero(callh);
		  free(text);
		  return 0;
	    }
      }

	/* Return the number of character read. */
      val.format = vpiIntVal;
      val.value.integer = strlen(text);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

	/* Return the characters to the register. */
      val.format = vpiStringVal;
      val.value.str = text;
      vpi_put_value(varh, &val, 0, vpiNoDelay);
      free(text);

      return 0;
}

static PLI_INT32 sys_fread_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

	/* We must have at least two arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Check that the first required argument is an integral variable or memory. */
      if (! is_int_var_or_mem(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument must be an integral variable or memory.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Check that the second required argument is numeric (a fd). */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a second (file descriptor) argument.\n",
	               name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      if (! is_numeric_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's second argument must be numeric.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/*
	 * If given check that the third argument is numeric (start).
	 *
	 * Technically you can give the fourth argument (count) with
	 * out a third argument (start), but Icarus does not currently
	 * support missing function arguments!
	 */
      arg = vpi_scan(argv);
      if (arg) {
	    if (! is_numeric_obj(arg)) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's third argument must be numeric.\n", name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }

	      /* If given check that the fourth argument is numeric (count). */
	    arg = vpi_scan(argv);
	    if (arg) {
		  if (! is_numeric_obj(arg)) {
			vpi_printf("ERROR: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			vpi_printf("%s's fourth argument must be numeric.\n",
			           name);
			vpip_set_return_value(1);
			vpi_control(vpiFinish, 1);
		  }

		    /* Make sure there are no extra arguments. */
		  check_for_extra_args(argv, callh, name, "four arguments", 1);
	    }
      }

      return 0;
}

/*
 * The pattern here is get the current vector, load the new bits on
 * top of the old ones and then put the modified vector. We need the
 * "get" first so that if we run out of bits in the file we keep the
 * original ones.
 */
static unsigned fread_word(FILE *fp, vpiHandle word,
                           unsigned words, unsigned bpe, s_vpi_vecval *vector)
{
      int bidx;
      s_vpi_value val;
      struct t_vpi_vecval *cur = &vector[words-1];
      unsigned rtn = 0;

	/* Get the current bits from the register and copy them to
	 * my local vector. */
      val.format = vpiVectorVal;
      vpi_get_value(word, &val);
      for (bidx = 0; (unsigned)bidx < words; bidx += 1) {
	    vector[bidx].aval = val.value.vector[bidx].aval;
	    vector[bidx].bval = val.value.vector[bidx].bval;
      }

	/* Copy the bytes to the local vector MSByte first. */
      for (bidx = bpe-1; bidx >= 0; bidx -= 1) {
	    unsigned clr_mask, bnum;
	    int byte = fgetc(fp);
	    if (byte == EOF) break;
	      /* Clear the current byte and load the new value. */
	    bnum = bidx % 4;
	    clr_mask = ~(0xff << bnum*8);
	    cur->aval &= clr_mask;
	    cur->bval &= clr_mask;
	    cur->aval |= byte << bnum*8;
	    rtn += 1;
	    if (bnum == 0) cur -= 1;
      }

	/* Put the updated bits into the register. */
      val.value.vector = vector;
      vpi_put_value(word, &val, 0, vpiNoDelay);

      return rtn;
}

static PLI_INT32 sys_fread_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg, mem_reg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      PLI_INT32 start, count, width, rtn;
      unsigned is_mem, bpe, words;
      FILE *fp;
      s_vpi_vecval *vector;
      errno = 0;

	/* Get the register/memory. */
      mem_reg = vpi_scan(argv);
	/* Get the file descriptor. */
      arg = vpi_scan(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

	/* Return 0 if this is not a valid fd. */
      fp = vpi_get_file(fd_mcd);
      if (!fp) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n",
	               (unsigned int)fd_mcd, name);
	    errno = EBADF;
	    val.format = vpiIntVal;
	    val.value.integer = 0;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    vpi_free_object(argv);
	    return 0;
      }

	/* Are we reading into a memory? */
      if (vpi_get(vpiType, mem_reg) == vpiMemory) is_mem = 1;
      else is_mem = 0;

	/* We only need to get these for memories. */
      if (is_mem) {
	    PLI_INT32 left, right, max, min;

	      /* Get the left and right memory address. */
	    val.format = vpiIntVal;
	    vpi_get_value(vpi_handle(vpiLeftRange, mem_reg), &val);
	    left = val.value.integer;
	    val.format = vpiIntVal;
	    vpi_get_value(vpi_handle(vpiRightRange, mem_reg), &val);
	    right = val.value.integer;
	    max = (left > right) ? left : right;
	    min = (left < right) ? left : right;

	      /* Get the starting address (optional). */
	    arg = vpi_scan(argv);
	    if (arg) {
		  val.format = vpiIntVal;
		  vpi_get_value(arg, &val);
		  start = val.value.integer;
		  if (start < min || start > max) {
			vpi_printf("WARNING: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			vpi_printf("%s's start argument (%d) is outside "
			           "memory range [%d:%d].\n", name, (int)start,
			           (int)left, (int)right);
			val.format = vpiIntVal;
			val.value.integer = 0;
			vpi_put_value(callh, &val, 0, vpiNoDelay);
			vpi_free_object(argv);
			return 0;
		  }

		    /* Get the count (optional). */
		  arg = vpi_scan(argv);
		  if (arg) {
			val.format = vpiIntVal;
			vpi_get_value(arg, &val);
			count = val.value.integer;
			if (count > max-start+1) {
			      vpi_printf("WARNING: %s:%d: ",
			                 vpi_get_str(vpiFile, callh),
			                 (int)vpi_get(vpiLineNo, callh));
			      vpi_printf("%s's count argument (%d) is too "
			                 "large for start (%d) and memory "
			                 "range [%d:%d].\n", name, (int)count,
			                 (int)start, (int)left, (int)right);
			      count = max - start + 1;
			}
			vpi_free_object(argv);
		  } else {
			count = max - start + 1;
		  }
	    } else {
		  start = min;
		  count = max - min + 1;
	    }
	    width = vpi_get(vpiSize, vpi_handle_by_index(mem_reg, start));
      } else {
	    start = 0;
	    count = 1;
	    width = vpi_get(vpiSize, mem_reg);
	    vpi_free_object(argv);
      }

      assert(width > 0);
      words = (width - 1)/32 + 1;
      vector = calloc(words, sizeof(s_vpi_vecval));
      bpe = (width+7)/8;

      assert(count >= 0);
      if (is_mem) {
	    unsigned idx;
	    rtn = 0;
	    for (idx = 0; idx < (unsigned)count; idx += 1) {
		  vpiHandle word;
		  word = vpi_handle_by_index(mem_reg, start+(signed)idx);
		  rtn += fread_word(fp, word, words, bpe, vector);
		  if (feof(fp)) break;
	    }
      } else {
	    rtn = fread_word(fp, mem_reg, words, bpe, vector);
      }
      free(vector);

	/* Return the number of bytes read. */
      val.format = vpiIntVal;
      val.value.integer = rtn;
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 sys_ungetc_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      FILE *fp;
      int chr;
      errno = 0;

	/* Get the character. */
      arg = vpi_scan(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      chr = val.value.integer;

	/* Get the file/MC descriptor. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

	/* Return EOF if this is not a valid fd. */
      fp = vpi_get_file(fd_mcd);
      if (!fp) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n",
	               (unsigned int)fd_mcd, name);
	    errno = EBADF;
	    val.format = vpiIntVal;
	    val.value.integer = EOF;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* ungetc the character and return the result. */
      val.format = vpiIntVal;
      val.value.integer = ungetc(chr, fp);
      if (val.value.integer != EOF) val.value.integer = 0;
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 sys_fseek_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

	/* Check that there are three numeric arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires three arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Check that the first argument is numeric. */
      if (! is_numeric_obj(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument must be numeric.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Check that the second argument exists and is numeric. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a second (numeric) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      if (! is_numeric_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's second argument must be numeric.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Check that the third argument exists and is numeric. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a third (numeric) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      if (! is_numeric_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's third argument must be numeric.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "three arguments", 0);

      return 0;
}

static PLI_INT32 sys_fseek_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      PLI_INT32 offset, oper;
      FILE *fp;
      errno = 0;

	/* Get the file pointer. */
      arg = vpi_scan(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

	/* Get the offset. */
      arg = vpi_scan(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      offset = val.value.integer;

	/* Get the operation. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      oper = val.value.integer;

	/* Check that the operation is in the valid range. */
      switch (oper) {
	case 0:
	    oper = SEEK_SET;
	    break;
	case 1:
	    oper = SEEK_CUR;
	    break;
	case 2:
	    oper = SEEK_END;
	    break;
	default:
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's operation must be 0, 1 or 2 given %d.\n",
	               name, (int)oper);
	    oper = -1; /* An invalid argument value. */
      }

	/* Return EOF if this is not a valid fd. */
      fp = vpi_get_file(fd_mcd);
      if (!fp) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n",
	               (unsigned int)fd_mcd, name);
	    errno = EBADF;
	    val.format = vpiIntVal;
	    val.value.integer = EOF;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    return 0;
      }

      val.format = vpiIntVal;
#if defined(__GNUC__)
      val.value.integer = fseek(fp, offset, oper);
#else
      if (oper < 0) {
	    val.value.integer = EOF;
	    errno = EINVAL;
      } else
	    val.value.integer = fseek(fp, offset, oper);
#endif
      vpi_put_value(callh, &val, 0 , vpiNoDelay);

      return 0;
}

static PLI_INT32 sys_common_fd_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;
      s_vpi_value val;
      PLI_UINT32 fd_mcd;
      FILE *fp;
      errno = 0;

	/* Get the file pointer. */
      arg = vpi_scan(argv);
      vpi_free_object(argv);
      val.format = vpiIntVal;
      vpi_get_value(arg, &val);
      fd_mcd = val.value.integer;

	/* Return EOF if this is not a valid fd. */
      fp = vpi_get_file(fd_mcd);
      if (!fp) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n",
	               (unsigned int)fd_mcd, name);
	    errno = EBADF;
	    val.format = vpiIntVal;
	    val.value.integer = EOF;
	    vpi_put_value(callh, &val, 0, vpiNoDelay);
	    return 0;
      }

      val.format = vpiIntVal;
      switch (name[4]) {
	  case 'l':  /* $ftell() */
	    val.value.integer = ftell(fp);
	    break;
	  case 'f':  /* $feof() is from 1264-2005*/
	    val.value.integer = feof(fp);
	    break;
	  case 'i':  /* $rewind() */
	    val.value.integer = fseek(fp, 0L, SEEK_SET);
	    break;
	  case 't':  /* $fgetc() */
	    val.value.integer = fgetc(fp);
	    break;
	  default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s cannot be processed with this routine.\n", name);
	    assert(0);
	    break;
      }
      vpi_put_value(callh, &val, 0 , vpiNoDelay);

      return 0;
}

/*
 * Implement the $ferror system function.
 */
static PLI_INT32 sys_ferror_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;
      vpiHandle arg;

      argv = vpi_iterate(vpiArgument, callh);

	/*
	 * Check that there are two arguments and that the first is
	 * numeric and that the second is a 640 bit or larger variable.
	 *
	 * The parser requires that a function have at least one argument,
	 * so argv should always be defined with one argument.
	 */
      assert(argv);

      if (! is_numeric_obj(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's fd (first) argument must be numeric.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Check that the second argument is given and that it is a 640 bit
	 * or larger variable. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a second (variable) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      if (! is_variable(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's second argument must be a variable (>=640 bits).\n",
	                name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      } else if ((vpi_get(vpiSize, arg) < 640) && (vpi_get(vpiType, arg) != vpiStringVar)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's second argument must have 640 bit or more.\n",
	               name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "two arguments", 0);

      return 0;
}

static PLI_INT32 sys_ferror_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle var;
      s_vpi_value val;
      char *msg;
      PLI_INT32 size;
      unsigned chars;
      PLI_UINT32 fd_mcd;

	/* Get the file pointer. */
      val.format = vpiIntVal;
      vpi_get_value(vpi_scan(argv), &val);
      fd_mcd = val.value.integer;

	/* Get the variable to put the string result and figure out how many
	 * characters it will hold. */
      var = vpi_scan(argv);
      size = vpi_get(vpiSize, var);
      chars = size / 8;
      vpi_free_object(argv);

	/* If we do not already have an error check that the fd is valid.
	 * The assumption is that the other routines have set errno to
	 * EBADF when they encounter a bad file descriptor, so we do not
	 * need to check here. We also need to special case this since
	 * $fopen() will return 0 (a bad file descriptor) when it has a
	 * problem (sets errno). */
      if (!errno && !vpi_get_file(fd_mcd) ) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("invalid file descriptor (0x%x) given to %s.\n",
	               (unsigned int)fd_mcd, name);
	    errno = EBADF;
      }

	/* Return the error code. */
      val.format = vpiIntVal;
      val.value.integer = errno;
      vpi_put_value(callh, &val, 0, vpiNoDelay);

	/* Only return the number of characters that will fit in the variable. */
      if (vpi_get(vpiType, var) == vpiStringVar) {
	    if (errno != 0) msg = strdup(strerror(errno));
	    else msg = strdup("");
      } else {
	    msg = (char *) malloc(chars);
	    if (errno != 0) strncpy(msg, strerror(errno), chars-1);
	    else strncpy(msg, "", chars-1);
	    msg[chars-1] = '\0';
      }
      val.format = vpiStringVal;
      val.value.str = msg;
      vpi_put_value(var, &val, 0, vpiNoDelay);
      free(msg);

      return 0;
}

void sys_fileio_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      /*============================== fopen */
      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname      = "$fopen";
      tf_data.calltf      = sys_fopen_calltf;
      tf_data.compiletf   = sys_fopen_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$fopen";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fopenr */
      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname      = "$fopenr";
      tf_data.calltf      = sys_fopenrwa_calltf;
      tf_data.compiletf   = sys_one_string_arg_compiletf;
      tf_data.sizetf      = 0;
      tf_data.user_data   = "$fopenr";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fopenw */
      tf_data.tfname      = "$fopenw";
      tf_data.user_data   = "$fopenw";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fopena */
      tf_data.tfname      = "$fopena";
      tf_data.user_data   = "$fopena";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fclose */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fclose";
      tf_data.calltf    = sys_fclose_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fclose";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fflush */
      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$fflush";
      tf_data.calltf    = sys_fflush_calltf;
      tf_data.compiletf = sys_one_opt_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fflush";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fgetc */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$fgetc";
      tf_data.calltf    = sys_common_fd_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fgetc";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fgets */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$fgets";
      tf_data.calltf    = sys_fgets_calltf;
      tf_data.compiletf = sys_fgets_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fgets";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fread */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$fread";
      tf_data.calltf    = sys_fread_calltf;
      tf_data.compiletf = sys_fread_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fread";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== ungetc */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$ungetc";
      tf_data.calltf    = sys_ungetc_calltf;
      tf_data.compiletf = sys_two_numeric_args_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ungetc";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== ftell */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$ftell";
      tf_data.calltf    = sys_common_fd_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ftell";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== fseek */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$fseek";
      tf_data.calltf    = sys_fseek_calltf;
      tf_data.compiletf = sys_fseek_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fseek";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
      vpip_make_systf_system_defined(res);

      /*============================== rewind */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$rewind";
      tf_data.calltf    = sys_common_fd_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$rewind";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /*============================== ferror */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$ferror";
      tf_data.calltf    = sys_ferror_calltf;
      tf_data.compiletf = sys_ferror_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$ferror";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /* $feof() is from 1364-2005. */
      /*============================== feof */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$feof";
      tf_data.calltf    = sys_common_fd_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$feof";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /* Icarus specific. */
      /*============================== fputc */
      tf_data.type      = vpiSysFunc;
      tf_data.sysfunctype = vpiIntFunc;
      tf_data.tfname    = "$fputc";
      tf_data.calltf    = sys_fputc_calltf;
      tf_data.compiletf = sys_two_numeric_args_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$fputc";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
