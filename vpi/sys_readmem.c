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
#ident "$Id: sys_readmem.c,v 1.8 2001/12/01 02:40:10 steve Exp $"
#endif

# include "config.h"

# include  "vpi_user.h"
# include  <string.h>
# include  <stdlib.h>
# include  <stdio.h>
# include  <assert.h>
# include  "sys_readmem_lex.h"

static int sys_readmem_calltf(char*name)
{
      int code;
      int wwid;
      char*path;
      FILE*file;
      unsigned addr;
      s_vpi_value value;
      vpiHandle words;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);
      vpiHandle mitem;

      if (item == 0) {
	    vpi_printf("%s: file name parameter missing.\n", name);
	    return 0;
      }

      if (vpi_get(vpiType, item) != vpiConstant) {
	    vpi_printf("ERROR: %s parameter must be a constant\n", name);
	    vpi_free_object(argv);
	    return 0;
      }

      if (vpi_get(vpiConstType, item) != vpiStringConst) {
	    vpi_printf("ERROR: %s parameter must be a constant\n", name);
	    vpi_free_object(argv);
	    return 0;
      }

      value.format = vpiStringVal;
      vpi_get_value(item, &value);
      path = strdup(value.value.str);

	/* Get and check the second paramter. It must be a memory. */
      mitem = vpi_scan(argv);
      if (mitem == 0) {
	    vpi_printf("%s: Missing memory parameter\n", name);
	    free(path);
	    return 0;
      }

      if (vpi_get(vpiType, mitem) != vpiMemory) {
	    vpi_printf("%s: Second parameter must be a memory.\n", name);
	    free(path);
	    vpi_free_object(argv);
	    return 0;
      }

	/* XXXX remaining parameters not supported. */
      vpi_free_object(argv);

	/* Open the data file. */
      file = fopen(path, "r");
      if (file == 0) {
	    vpi_printf("%s: Unable to open %s for reading.\n", name, path);
	    free(path);
	    return 0;
      }

      words = vpi_iterate(vpiMemoryWord, mitem);
      assert(words);

      item = vpi_scan(words);
      wwid = vpi_get(vpiSize, item);

      value.format = vpiVectorVal;
      value.value.vector = calloc((wwid+31)/32, sizeof (s_vpi_vecval));
      if (strcmp(name,"$readmemb") == 0)
	    sys_readmem_start_file(file, 1, wwid, value.value.vector);
      else
	    sys_readmem_start_file(file, 0, wwid, value.value.vector);

      addr = 0;

      while ((code = readmemlex()) != 0) {
	    switch (code) {
		case MEM_ADDRESS:
		  if (addr > value.value.vector->aval) {
			vpi_free_object(words);
			words = vpi_iterate(vpiMemoryWord, mitem);
			item = vpi_scan(words);
			addr = 0;
		  }
		  while (item && addr < value.value.vector->aval) {
			item = vpi_scan(words);
			addr += 1;
		  }
		  break;
		case MEM_WORD:
		  if (item) {
			vpi_put_value(item, &value, 0, vpiNoDelay);
			item = vpi_scan(words);
			addr += 1;
		  } else {
			vpi_printf("%s(%s): too much data (addr=0x%x)\n", 
				   name, path, addr);
			goto bailout;
		  }
		  break;
		default:
		  vpi_printf("Huh?! (%d)\n", code);
		  break;
	    }
      }

  bailout:
      if (item) vpi_free_object(words);
      free(path);
      free(value.value.vector);
      fclose(file);
      return 0;
}

static int sys_writemem_calltf(char*name)
{
      int wwid;
      char*path;
      FILE*file;
      unsigned cnt = 0;
      s_vpi_value value;
      vpiHandle words;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);

      if (item == 0) {
	    vpi_printf("%s: file name parameter missing.\n", name);
	    return 0;
      }

      if (vpi_get(vpiType, item) != vpiConstant) {
	    vpi_printf("ERROR: %s parameter must be a constant\n", name);
	    vpi_free_object(argv);
	    return 0;
      }

      if (vpi_get(vpiConstType, item) != vpiStringConst) {
	    vpi_printf("ERROR: %s parameter must be a constant\n", name);
	    vpi_free_object(argv);
	    return 0;
      }

      value.format = vpiStringVal;
      vpi_get_value(item, &value);
      path = strdup(value.value.str);

	/* Get and check the second paramter. It must be a memory. */
      item = vpi_scan(argv);
      if (item == 0) {
	    vpi_printf("%s: Missing memory parameter\n", name);
	    free(path);
	    return 0;
      }

      if (vpi_get(vpiType, item) != vpiMemory) {
	    vpi_printf("%s: Second parameter must be a memory.\n", name);
	    free(path);
	    vpi_free_object(argv);
	    return 0;
      }

	/* XXXX remaining parameters not supported. */
      vpi_free_object(argv);

	/* Open the data file. */
      file = fopen(path, "w");
      if (file == 0) {
	    vpi_printf("%s: Unable to open %s for writeing.\n", name, path);
	    free(path);
	    return 0;
      }

      words = vpi_iterate(vpiMemoryWord, item);
      assert(words);

      wwid = vpi_get(vpiSize, item);

      value.format = vpiIntVal;

      while ((item = vpi_scan(words))) {
	    if (cnt%16 == 0)
		  fprintf(file, "// 0x%08x\n", cnt);
	    cnt += 1;
	    vpi_get_value(item, &value);
	    fprintf(file, "%lx\n", (unsigned long)value.value.integer);
      }

      fclose(file);
      return 0;
}

void sys_readmem_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$readmemh";
      tf_data.calltf    = sys_readmem_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$readmemh";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$readmemb";
      tf_data.calltf    = sys_readmem_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$readmemb";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$writememh";
      tf_data.calltf    = sys_writemem_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$writememh";
      vpi_register_systf(&tf_data);
}

/*
 * $Log: sys_readmem.c,v $
 * Revision 1.8  2001/12/01 02:40:10  steve
 *  Support addresses in readmemh.
 *
 * Revision 1.7  2001/11/09 03:39:21  steve
 *  Support $writememh
 *
 * Revision 1.6  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.5  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.4  2000/01/23 23:54:36  steve
 *  Compile time problems with vpi_user.h
 *
 * Revision 1.3  1999/12/15 04:35:34  steve
 *  Add readmemb.
 *
 * Revision 1.2  1999/12/15 04:02:38  steve
 *  Excess warning.
 *
 * Revision 1.1  1999/12/15 04:01:14  steve
 *  Add the VPI implementation of $readmemh.
 *
 */

