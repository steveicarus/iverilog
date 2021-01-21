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
# include  <ctype.h>
# include  <string.h>
# include  <stdlib.h>
# include  <stdio.h>
# include  <assert.h>
# include  "sys_readmem_lex.h"
# include  <sys/stat.h>
# include  "ivl_alloc.h"

char **search_list = NULL;
unsigned sl_count = 0;

static void get_mem_params(vpiHandle argv, vpiHandle callh, const char *name,
                           char **fname, vpiHandle *mitem,
                           vpiHandle *start_item, vpiHandle *stop_item)
{
	/* Get the first parameter (file name). */
      *fname = get_filename(callh, name, vpi_scan(argv));

	/* Get the second parameter (memory). */
      *mitem = vpi_scan(argv);

      /* Get optional third parameter (start address). */
      *start_item = vpi_scan(argv);
      if (*start_item) {
	    /* Warn the user if they gave a real value for the start
	     * address. */
	    switch (vpi_get(vpiType, *start_item)) {
		case vpiConstant:
		case vpiParameter:
		  if (vpi_get(vpiConstType, *start_item) != vpiRealConst) break;
		  // fallthrough
		case vpiRealVar:
		  vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's third argument (start address) is a real "
		             "value.\n", name);
	    }

	    /* Get optional fourth parameter (finish address). */
	    *stop_item = vpi_scan(argv);
	    if (*stop_item) {
		  /* Warn the user if they gave a real value for the finish
		   * address. */
		  switch (vpi_get(vpiType, *stop_item)) {
		      case vpiConstant:
		      case vpiParameter:
			if (vpi_get(vpiConstType, *stop_item) != vpiRealConst) {
			      break;
			}
			// fallthrough
		      case vpiRealVar:
			vpi_printf("WARNING: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			vpi_printf("%s's fourth argument (finish address) is a "
			           "real value.\n", name);
		  }
		  vpi_free_object(argv);
	    }
      } else {
	   *stop_item = 0;
      }
}

static int process_params(vpiHandle mitem,
                          vpiHandle start_item, vpiHandle stop_item,
                          vpiHandle callh, const char *name,
                          int *start_addr, int *stop_addr, int *addr_incr,
                          int *min_addr, int *max_addr)
{
      s_vpi_value val;
      int left_addr, right_addr;

	/* Get left addr of memory */
      val.format = vpiIntVal;
      vpi_get_value(vpi_handle(vpiLeftRange, mitem), &val);
      left_addr = val.value.integer;

	/* Get right addr of memory */
      val.format = vpiIntVal;
      vpi_get_value(vpi_handle(vpiRightRange, mitem), &val);
      right_addr = val.value.integer;

	/* Get start_addr, stop_addr and addr_incr */
      if (! start_item) {
	  *start_addr = left_addr<right_addr ? left_addr  : right_addr;
	  *stop_addr  = left_addr<right_addr ? right_addr : left_addr;
	  *addr_incr = 1;
      } else {
	  val.format = vpiIntVal;
	  vpi_get_value(start_item, &val);
	  *start_addr = val.value.integer;

	  if (! stop_item) {
	      *stop_addr = left_addr<right_addr ? right_addr : left_addr;
	      *addr_incr = 1;
	  } else {
	      val.format = vpiIntVal;
	      vpi_get_value(stop_item, &val);
	      *stop_addr = val.value.integer;

	      *addr_incr = *start_addr<*stop_addr ? 1 : -1;
	  }
      }

	/* Find the minimum and maximum address. */
      *min_addr = *start_addr<*stop_addr ? *start_addr : *stop_addr ;
      *max_addr = *start_addr<*stop_addr ? *stop_addr  : *start_addr;

	/* If the range is not fully specified and the left address is
	 * greater than the right address. Print a warning that this
	 * code follows 1364-2005.
	 *
	 * If we passed a generation flag we could do the correct thing
	 * for 1364-1995 and 1364-2001 instead of this general warning
	 * or we could only show the warning when using 2001/1995.
	 */
      if (!stop_item && (left_addr > right_addr)) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s: The behaviour for reg[...] mem[N:0]; %s(\"...\", mem);"
		       " changed in the 1364-2005 standard. To avoid ambiguity,"
		       " use mem[0:N] or explicit range parameters"
		       " %s(\"...\", mem, start, stop);. Defaulting to 1364-2005"
		       " behavior.\n",
		       name, name, name);

      }

	/* Check that start_addr and stop_addr are within the memory
	   range */
      if (left_addr < right_addr) {
	  if (*start_addr < left_addr || *start_addr > right_addr) {
	      vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	                 (int)vpi_get(vpiLineNo, callh));
	      vpi_printf("%s: Start address %d is out of bounds for memory "
	                 "\'%s[%d:%d]\'!\n", name, *start_addr,
	                 vpi_get_str(vpiFullName, mitem),
	                 left_addr, right_addr);
	      return 1;
	  }

	  if (*stop_addr < left_addr || *stop_addr > right_addr) {
	      vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	                 (int)vpi_get(vpiLineNo, callh));
	      vpi_printf("%s: Finish address %d is out of bounds for memory "
	                 "\'%s[%d:%d]\'!\n", name, *stop_addr,
	                 vpi_get_str(vpiFullName, mitem),
	                 left_addr, right_addr);
	      return 1;
	  }
      } else {
	  if (*start_addr < right_addr || *start_addr > left_addr) {
	      vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	                 (int)vpi_get(vpiLineNo, callh));
	      vpi_printf("%s: Start address %d is out of bounds for memory "
	                 "\'%s[%d:%d]\'!\n", name, *start_addr,
	                 vpi_get_str(vpiFullName, mitem),
	                 left_addr, right_addr);
	      return 1;
	  }

	  if (*stop_addr < right_addr || *stop_addr > left_addr) {
	      vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	                 (int)vpi_get(vpiLineNo, callh));
	      vpi_printf("%s: Finish address %d is out of bounds for memory "
	                 "\'%s[%d:%d]\'!\n", name, *stop_addr,
	                 vpi_get_str(vpiFullName, mitem),
	                 left_addr, right_addr);
	      return 1;
	  }
      }
      return 0;
}

static PLI_INT32 sys_mem_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

      /* Check that there is a file name argument. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      if (! is_string_obj(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument must be a file name (string).\n",
	               name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

      /* Check that there is a memory argument. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a second (memory) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      if (vpi_get(vpiType, arg) != vpiMemory) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's second argument must be a memory.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

      /* Check if there is a starting address argument. */
      arg = vpi_scan(argv);
      if (! arg) return 0;

      if (! is_numeric_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's third argument must be a start address "
	               "(numeric).\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

      /* Check if there is a finish address argument. */
      arg = vpi_scan(argv);
      if (! arg) return 0;

      if (! is_numeric_obj(arg)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's fourth argument must be a finish address "
	               "(numeric).\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "four arguments", 1);

      return 0;
}

static PLI_INT32 sys_readmem_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      int code, wwid, addr;
      FILE*file;
      char *fname = 0;
      s_vpi_value value;
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle mitem = 0;
      vpiHandle start_item = 0;
      vpiHandle stop_item = 0;

      /* start_addr and stop_addr are the parameters given to $readmem in the
	 Verilog code. When not specified, start_addr is equal to the lower of
	 the [left,right]_addr and stop_addr is equal to the higher of the
	 [left,right]_addr. */
      int start_addr, stop_addr, addr_incr;

      /* min_addr and max_addr are equal to start_addr and stop_addr if
	 start_addr<stop_addr or vice versa if not... */
      int min_addr, max_addr;

      /* This is the number of words that we need from the memory. */
      unsigned word_count;

      /*======================================== Get parameters */

      get_mem_params(argv, callh, name,
                     &fname, &mitem, &start_item, &stop_item);
      if (fname == 0) return 0;

      /*======================================== Process parameters */

      if (process_params(mitem, start_item, stop_item, callh, name,
                         &start_addr, &stop_addr, &addr_incr,
                         &min_addr, &max_addr)) {
	    free(fname);
	    return 0;
      }

	/* Open the data file. */
      file = fopen(fname, "r");
	/* Check to see if we have other directories to look for this file. */
      if (file == 0 && sl_count > 0 && fname[0] != '/') {
	    unsigned idx;
	    char path[4096];

	    for (idx = 0; idx < sl_count; idx += 1) {
		  snprintf(path, sizeof(path), "%s/%s",
		           search_list[idx], fname);
		  path[sizeof(path)-1] = 0;
		  if ((file = fopen(path, "r"))) break;
	    }
      }
      if (file == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s: Unable to open %s for reading.\n", name, fname);
	    free(fname);
	    return 0;
      }

	/* We need this many words from the file. */
      word_count = max_addr-min_addr+1;

      wwid = vpi_get(vpiSize, vpi_handle_by_index(mitem, min_addr));

      /* variable that will be used by the lexer to pass values
	 back to this code */
      value.format = vpiVectorVal;
      value.value.vector = calloc((wwid+31)/32, sizeof(s_vpi_vecval));

      /* Configure the readmem lexer */
      if (strcmp(name,"$readmemb") == 0)
	    sys_readmem_start_file(callh, file, 1, wwid, value.value.vector);
      else
	    sys_readmem_start_file(callh, file, 0, wwid, value.value.vector);

      /*======================================== Read memory file */

      /* Run through the input file and store the new contents in the memory */
      addr = start_addr;
      while ((code = readmemlex()) != 0) {
	  switch (code) {
	  case MEM_ADDRESS:
	      addr = value.value.vector->aval;
	      if (addr < min_addr || addr > max_addr) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s(%s): address (0x%x) is out of range "
		             "[0x%x:0x%x]\n",
			     name, fname, addr, start_addr, stop_addr);
		  goto bailout;
	      }
		/* if there is an address in the memory file, then
		   turn off any possible warnings about not having
		   enough words to load the memory. This is standard
		   behavior from 1364-2005. */
	      word_count = 0;
	      break;

	  case MEM_WORD:
	      if (addr >= min_addr && addr <= max_addr) {
		  vpiHandle word_index;
		  word_index = vpi_handle_by_index(mitem, addr);
		  assert(word_index);
		  vpi_put_value(word_index, &value, 0, vpiNoDelay);

		  if (word_count > 0) word_count -= 1;
	      } else {
		  vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s(%s): Too many words in the file for the "
		             "requested range [%d:%d].\n",
			     name, fname, start_addr, stop_addr);
		  goto bailout;
	      }

	      addr += addr_incr;
	      break;

	  case MEM_ERROR:
	      vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	                 (int)vpi_get(vpiLineNo, callh));
	      vpi_printf("%s(%s): Invalid input character: %s\n", name,
	                 fname, readmem_error_token);
	      goto bailout;
	      break;

	  default:
	      assert(0);
	      break;
	  }
      }

	/* Print a warning if there are not enough words in the data file. */
      if (word_count > 0) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s(%s): Not enough words in the file for the "
		       "requested range [%d:%d].\n", name, fname,
		       start_addr, stop_addr);
      }

 bailout:
      free(value.value.vector);
      free(fname);
      fclose(file);
      destroy_readmem_lexor();
      return 0;
}

static PLI_INT32 free_readmempath(p_cb_data cb_data)
{
      unsigned idx;

      (void)cb_data; /* Parameter is not used. */
      for(idx = 0; idx < sl_count; idx += 1) {
	    free(search_list[idx]);
      }
      free(search_list);
      search_list = NULL;
      sl_count = 0;
      return 0;
}

static PLI_INT32 sys_readmempath_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle paths = vpi_scan(argv);
      s_vpi_value val;
      unsigned len, idx;
      char *path;

      vpi_free_object(argv);

	/* Get the search path string. */
      val.format = vpiStringVal;
      vpi_get_value(paths, &val);

	/* Verify that we have a string and that it is not NULL. */
      if (val.format != vpiStringVal || !*(val.value.str)) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	                (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's argument (%s) is not a valid string.\n",
	               name, vpi_get_str(vpiType, paths));
	    return 0;
      }

	/*
	 * Verify that the search path is composed of only printable
	 * characters.
	 */
      len = strlen(val.value.str);
      for (idx = 0; idx < len; idx++) {
	    if (! isprint((int)val.value.str[idx])) {
		  char msg[64];
		  char *esc_path = as_escaped(val.value.str);
		  snprintf(msg, sizeof(msg), "WARNING: %s:%d:",
		           vpi_get_str(vpiFile, callh),
		           (int)vpi_get(vpiLineNo, callh));
		  msg[sizeof(msg)-1] = 0;
		  vpi_printf("%s %s's argument contains non-printable "
		             "characters.\n", msg, name);
		  vpi_printf("%*s \"%s\"\n", (int) strlen(msg), " ", esc_path);
		  free(esc_path);
		  return 0;
	    }
      }

	/* Clear the old list before creating the new list. */
      free_readmempath(NULL);

	/*
	 * Break the string into individual paths and add them to the list.
	 * Print a warning if the path is not valid.
	 */
      for (path = strtok(val.value.str, ":"); path; path = strtok(NULL, ":")) {
	    int res;
	    struct stat sb;

	      /* Warn the user if the path is not valid. */
	    res = stat(path, &sb);
	    if (res == 0) {
		  if (!S_ISDIR(sb.st_mode)) {
			vpi_printf("WARNING: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			vpi_printf("%s's path element \"%s\" is not a "
			           "directory!\n", name, path);
			continue;
		  }
	    } else {
		  vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s could not find directory \"%s\"!\n",
		             name, path);
		  continue;
	    }

	      /* Add a valid search path element to the list. */
	    sl_count += 1;
	    search_list = (char **) realloc(search_list,
	                                    sizeof(char **)*sl_count);
	    search_list[sl_count-1] = strdup(path);
      }

      return 0;
}

static PLI_INT32 sys_writemem_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      int addr;
      FILE*file;
      char*fname = 0;
      unsigned cnt;
      s_vpi_value value;
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle mitem = 0;
      vpiHandle start_item = 0;
      vpiHandle stop_item = 0;

      int start_addr, stop_addr, addr_incr;
      int min_addr, max_addr; // Not used in this routine.

      /*======================================== Get parameters */

      get_mem_params(argv, callh, name,
                     &fname, &mitem, &start_item, &stop_item);

      if (fname == 0) return 0;

      /*======================================== Process parameters */

      if (process_params(mitem, start_item, stop_item, callh, name,
                         &start_addr, &stop_addr, &addr_incr,
                         &min_addr, &max_addr)) {
	    free(fname);
	    return 0;
      }

      /* Open the data file. */
      file = fopen(fname, "w");
      if (file == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s: Unable to open %s for writing.\n", name, fname);
	    free(fname);
	    return 0;
      }

      if (strcmp(name,"$writememb")==0) value.format = vpiBinStrVal;
      else value.format = vpiHexStrVal;

      /*======================================== Write memory file */

      cnt = 0;
      for(addr=start_addr; addr!=stop_addr+addr_incr; addr+=addr_incr, ++cnt) {
	  vpiHandle word_index;

	  if (cnt%16 == 0) fprintf(file, "// 0x%08x\n", cnt);

	  word_index = vpi_handle_by_index(mitem, addr);
	  assert(word_index);
	  vpi_get_value(word_index, &value);
	  fprintf(file, "%s\n", value.value.str);
      }

      fclose(file);
      free(fname);
      return 0;
}

void sys_readmem_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;
      s_cb_data cb_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$readmemh";
      tf_data.calltf    = sys_readmem_calltf;
      tf_data.compiletf = sys_mem_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$readmemh";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$readmemb";
      tf_data.calltf    = sys_readmem_calltf;
      tf_data.compiletf = sys_mem_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$readmemb";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$readmempath";
      tf_data.calltf    = sys_readmempath_calltf;
      tf_data.compiletf = sys_one_string_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$readmempath";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$writememh";
      tf_data.calltf    = sys_writemem_calltf;
      tf_data.compiletf = sys_mem_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$writememh";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$writememb";
      tf_data.calltf    = sys_writemem_calltf;
      tf_data.compiletf = sys_mem_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$writememb";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      cb_data.reason = cbEndOfSimulation;
      cb_data.time = 0;
      cb_data.cb_rtn = free_readmempath;
      cb_data.user_data = "system";
      vpi_register_cb(&cb_data);
}
