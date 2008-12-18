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

# include "vpi_config.h"

# include  "vpi_user.h"
# include  <string.h>
# include  <stdlib.h>
# include  <stdio.h>
# include  <assert.h>
# include  "sys_readmem_lex.h"

static int check_integer_constant(char*name, vpiHandle handle)
{
    if (vpi_get(vpiType, handle) != vpiConstant){
	vpi_printf("ERROR: %s parameter must be a constant (vpiType=%d)\n",
		   name, vpi_get(vpiType, handle));
	return 0;
    }

    switch(vpi_get(vpiConstType, handle)){
    case vpiDecConst:
    case vpiBinaryConst:
    case vpiOctConst:
    case vpiHexConst:
	return 1;
	break;

	/* We rely on vpi_get_value for reals and strings to return a correct */
	/* integer value when this is requested. So only a warning is generated. */
    case vpiRealConst:
	vpi_printf("Warning: real supplied to %s instead of integer.\n", name);
	return 1;
	break;

    case vpiStringConst:
	vpi_printf("Warning: string supplied to %s instead of integer.\n", name);
	return 1;
	break;
    }

    /* switch statement covers all possibilities. Code should never come here... */
    assert(0);
    return 0;
}

/*
 * This function makes sure the handle is of an object that can get a
 * string value for a file name.
 */
static int check_file_name(const char*name, vpiHandle item)
{
      switch (vpi_get(vpiType, item)) {

	  case vpiConstant:
	    if (vpi_get(vpiConstType, item) != vpiStringConst) {
		  vpi_printf("ERROR: %s argument 1: file name argument "
			     "must be a string.\n", name);
		  return 0;
	    }
	    break;

	  case vpiParameter:
	    if (vpi_get(vpiConstType,item) != vpiStringConst) {
		  vpi_printf("ERROR: %s argument 1: Parameter %s is "
			     "not a string in this context.\n",
			     name, vpi_get_str(vpiName,item));
		  return 0;
	    }
	    break;

	  case vpiReg:
	    break;

	  default:
	    vpi_printf("ERROR: %s argument 1: must be a string\n", name);
	    return 0;
      }

      return 1;
}


static PLI_INT32 sys_readmem_calltf(PLI_BYTE8*name)
{
      int code;
      int wwid;
      char*path;
      char*mem_name;
      FILE*file;
      unsigned addr;
      s_vpi_value value;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);
      vpiHandle mitem;
      vpiHandle start_item;
      vpiHandle stop_item;
      vpiHandle left_range;
      vpiHandle right_range;
      vpiHandle word_index;

      /* These are left and right hand side parameters in the
	 declaration of the memory. */
      int left_addr, right_addr;

      /* start_addr and stop_addr are the parameters given to $readmem in the
	 Verilog code. When not specified, start_addr is equal to the lower of
	 the [left,right]_addr and stop_addr is equal to the higher of the
	 [left,right]_addr. */
      int start_addr, stop_addr, addr_incr;

      /* min_addr and max_addr are equal to start_addr and stop_addr if
	 start_addr<stop_addr or vice versa if not... */
      unsigned min_addr, max_addr;

	/* This is the number of words that we need from the memory. */
      unsigned word_count;


      /*======================================== Get parameters */

      if (item == 0) {
	    vpi_printf("%s: file name parameter missing.\n", name);
	    return 0;
      }

	/* Check then get the first argument, the file name. It is
	   possible that Verilog would right-justify a name to fit a
	   reg value to fit the reg width, so chop off leading white
	   space in the process. */
      if (check_file_name(name, item) == 0) {
	    vpi_free_object(argv);
	    return 0;
      }

      value.format = vpiStringVal;
      vpi_get_value(item, &value);
      path = strdup(value.value.str + strspn(value.value.str, " "));

	/* Get and check the second parameter. It must be a memory. */
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

      mem_name = vpi_get_str(vpiFullName, mitem);

      /* Get optional third parameter. It must be a constant. */
      start_item = vpi_scan(argv);
      if (start_item!=0){
	  if (check_integer_constant(name, start_item) == 0){
	      vpi_free_object(argv);
	      return 0;
	  }

	  /* Get optional forth parameter. It must be a constant. */
	  stop_item = vpi_scan(argv);
	  if (stop_item!=0){
	      if (check_integer_constant(name, stop_item) == 0){
		  vpi_free_object(argv);
		  return 0;
	      }

	      /* Check that there is no 5th parameter */
	      if (vpi_scan(argv) != 0){
		  vpi_printf("ERROR: %s accepts maximum 4 parameters!\n", name );
		  vpi_free_object(argv);
		  return 0;
	      }

	  }
      }
      else{
	  stop_item = 0;
      }

      /*======================================== Process parameters */

      /* Open the data file. */
      file = fopen(path, "r");
      if (file == 0) {
	    vpi_printf("%s: Unable to open %s for reading.\n", name, path);
	    free(path);
	    return 0;
      }

      /* Get left addr of memory */
      left_range = vpi_handle(vpiLeftRange, mitem);
      value.format = vpiIntVal;
      vpi_get_value(left_range, &value);
      left_addr = value.value.integer;

      /* Get right addr of memory */
      right_range = vpi_handle(vpiRightRange, mitem);
      value.format = vpiIntVal;
      vpi_get_value(right_range, &value);
      right_addr = value.value.integer;

      /* Get start_addr, stop_addr and addr_incr */
      if (start_item==0){
	  start_addr = left_addr<right_addr ? left_addr  : right_addr;
	  stop_addr  = left_addr<right_addr ? right_addr : left_addr;
	  addr_incr = 1;
      }
      else{
	  s_vpi_value value2;
	  value2.format = vpiIntVal;
	  vpi_get_value(start_item, &value2);
	  start_addr = value2.value.integer;

	  if (stop_item==0){
	      stop_addr = left_addr<right_addr ? right_addr : left_addr;
	      addr_incr = 1;
	  }
	  else{
	      s_vpi_value value3;
	      value3.format = vpiIntVal;
	      vpi_get_value(stop_item, &value3);
	      stop_addr = value3.value.integer;

	      addr_incr = start_addr<stop_addr ? 1 : -1;
	  }
      }

      min_addr = start_addr<stop_addr ? start_addr : stop_addr ;
      max_addr = start_addr<stop_addr ? stop_addr  : start_addr;

	/* We need this many words from the file. */
      word_count = max_addr-min_addr+1;

      /* Check that start_addr and stop_addr are within the memory
	 range */
      if (left_addr<right_addr){
	  if (start_addr<left_addr || start_addr > right_addr) {
	      vpi_printf("%s: Start address is out of bounds for memory \'%s\'!\n", name, mem_name);
	      return 0;
	  }

	  if (stop_addr<left_addr || stop_addr > right_addr) {
	      vpi_printf("%s: Stop address is out of bounds for memory \'%s\'!\n", name, mem_name);
	      return 0;
	  }
      }
      else{
	  if (start_addr<right_addr || start_addr > left_addr) {
	      vpi_printf("%s: Start address is out of bounds for memory \'%s\'!\n", name, mem_name);
	      return 0;
	  }

	  if (stop_addr<right_addr || stop_addr > left_addr) {
	      vpi_printf("%s: Stop address is out of bounds for memory \'%s\'!\n", name, mem_name);
	      return 0;
	  }
      }

      item = vpi_handle_by_index(mitem,min_addr);
      wwid = vpi_get(vpiSize, item);

      /* variable that will be uses by the lexer to pass values
	 back to this code */
      value.format = vpiVectorVal;
      value.value.vector = calloc((wwid+31)/32, sizeof(s_vpi_vecval));

      /* Configure the readmem lexer */
      if (strcmp(name,"$readmemb") == 0)
	  sys_readmem_start_file(file, 1, wwid, value.value.vector);
      else
	  sys_readmem_start_file(file, 0, wwid, value.value.vector);


      /*======================================== Read memory file */

      /* Run through the input file and store the new contents in the memory */
      addr = start_addr;
      while ((code = readmemlex()) != 0) {
	  switch (code) {
	  case MEM_ADDRESS:
	      addr = value.value.vector->aval;
		/* if there is an address in the memory file, then
		   turn off any possible warnings about not having
		   enough words to load the memory. This is standard
		   behavior. */
	      word_count = 0;
	      break;

	  case MEM_WORD:
	      if (addr >= min_addr && addr <= max_addr){
		  word_index = vpi_handle_by_index(mitem, addr);
		  assert(word_index);
		  vpi_put_value(word_index, &value, 0, vpiNoDelay);

		  if (word_count > 0)
			word_count -= 1;
	      }
	      else{
		  vpi_printf("%s(%s): address (0x%x) out of range (0x%x:0x%x)\n",
			     name, path, addr, start_addr, stop_addr);
		  goto bailout;
	      }

	      addr += addr_incr;
	      break;

	  default:
	      vpi_printf("Huh?! (%d)\n", code);
	      break;
	  }
      }

      if (word_count > 0)
	    vpi_printf("%s(%s): Not enough words in the read file "
		       "for requested range.\n", name, path);

 bailout:
      free(value.value.vector);
      free(path);
      fclose(file);
      return 0;
}

static PLI_INT32 sys_writemem_calltf(PLI_BYTE8*name)
{
      int wwid;
      char*path;
      char*mem_name;
      FILE*file;
      unsigned addr = 0;
      unsigned cnt = 0;
      s_vpi_value value;
      vpiHandle words;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item = vpi_scan(argv);
      vpiHandle mitem;
      vpiHandle start_item;
      vpiHandle stop_item;
      vpiHandle word_index;
      vpiHandle left_range;
      vpiHandle right_range;

      int left_addr, right_addr;
      int start_addr, stop_addr, addr_incr;
      int min_addr, max_addr;

      /*======================================== Get parameters */

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
	    vpi_printf("ERROR: %s parameter must be a string\n", name);
	    vpi_free_object(argv);
	    return 0;
      }

      value.format = vpiStringVal;
      vpi_get_value(item, &value);
      path = strdup(value.value.str);

	/* Get and check the second parameter. It must be a memory. */
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

      mem_name = vpi_get_str(vpiFullName, mitem);

      /* Get optional third parameter. It must be a constant. */
      start_item = vpi_scan(argv);
      if (start_item!=0){
	  if (check_integer_constant(name, start_item) == 0){
	      vpi_free_object(argv);
	      return 0;
	  }

	  /* Get optional forth parameter. It must be a constant. */
	  stop_item = vpi_scan(argv);
	  if (stop_item!=0){
	      if (check_integer_constant(name, stop_item) == 0){
		  vpi_free_object(argv);
		  return 0;
	      }

	      /* Check that there is no 5th parameter */
	      if (vpi_scan(argv) != 0){
		  vpi_printf("ERROR: %s accepts maximum 4 parameters!\n", name );
		  vpi_free_object(argv);
		  return 0;
	      }

	  }
      }
      else{
	  stop_item = 0;
      }

      /*======================================== Process parameters */

      /* Open the data file. */
      file = fopen(path, "w");
      if (file == 0) {
	    vpi_printf("%s: Unable to open %s for writing.\n", name, path);
	    free(path);
	    return 0;
      }

      /* Get left addr of memory */
      left_range = vpi_handle(vpiLeftRange, mitem);
      value.format = vpiIntVal;
      vpi_get_value(left_range, &value);
      left_addr = value.value.integer;

      /* Get right addr of memory */
      right_range = vpi_handle(vpiRightRange, mitem);
      value.format = vpiIntVal;
      vpi_get_value(right_range, &value);
      right_addr = value.value.integer;

      /* Get start_addr, stop_addr and addr_incr */
      if (start_item==0){
	  start_addr = left_addr<right_addr ? left_addr  : right_addr;
	  stop_addr  = left_addr<right_addr ? right_addr : left_addr;
	  addr_incr = 1;
      }
      else{
	  s_vpi_value value2;
	  value2.format = vpiIntVal;
	  vpi_get_value(start_item, &value2);
	  start_addr = value2.value.integer;

	  if (stop_item==0){
	      stop_addr = left_addr<right_addr ? right_addr : left_addr;
	      addr_incr = 1;
	  }
	  else{
	      s_vpi_value value3;
	      value3.format = vpiIntVal;
	      vpi_get_value(stop_item, &value3);
	      stop_addr = value3.value.integer;

	      addr_incr = start_addr<stop_addr ? 1 : -1;
	  }
      }

      min_addr = start_addr<stop_addr ? start_addr : stop_addr ;
      max_addr = start_addr<stop_addr ? stop_addr  : start_addr;

      /* Check that start_addr and stop_addr are within the memory
	 range */
      if (left_addr<right_addr){
	  if (start_addr<left_addr || start_addr > right_addr) {
	      vpi_printf("%s: Start address is out of bounds for memory \'%s\'!\n", name, mem_name);
	      return 0;
	  }

	  if (stop_addr<left_addr || stop_addr > right_addr) {
	      vpi_printf("%s: Stop address is out of bounds for memory \'%s\'!\n", name, mem_name);
	      return 0;
	  }
      }
      else{
	  if (start_addr<right_addr || start_addr > left_addr) {
	      vpi_printf("%s: Start address is out of bounds for memory \'%s\'!\n", name, mem_name);
	      return 0;
	  }

	  if (stop_addr<right_addr || stop_addr > left_addr) {
	      vpi_printf("%s: Stop address is out of bounds for memory \'%s\'!\n", name, mem_name);
	      return 0;
	  }
      }


      words = vpi_iterate(vpiMemoryWord, mitem);
      assert(words);

      item = vpi_scan(words);
      wwid = vpi_get(vpiSize, item);

      if (strcmp(name,"$writememb")==0){
	  value.format = vpiBinStrVal;
      }
      else{
	  value.format = vpiHexStrVal;
      }

      /*======================================== Write memory file */

      cnt=0;
      for(addr=start_addr; addr!=stop_addr+addr_incr; addr+=addr_incr, ++cnt){
	  if (cnt%16 == 0)
	      fprintf(file, "// 0x%08x\n", cnt);

	  word_index = vpi_handle_by_index(mitem, addr);
	  assert(word_index);
	  vpi_get_value(word_index, &value);
	  fprintf(file, "%s\n", value.value.str);
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

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$writememb";
      tf_data.calltf    = sys_writemem_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$writememb";
      vpi_register_systf(&tf_data);
}

