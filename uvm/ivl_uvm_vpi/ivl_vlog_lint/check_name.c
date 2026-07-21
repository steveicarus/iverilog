/**********************************************************************
 * VPI application for checking the naming convention of the variables 
 * and modules.
 *
 * Note : Typically such a piece of code is needed in lint checking.
 *
 *********************************************************************/

/* System headers:       */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vpi_user.h>

/**********************************************************************
 * check_var_decl(): 
 * Identifies all reg declarations in the design with -
 *       Names that are longer than 20 characters.
 *       Names that have more than 4 "_" in them.
 *       Names that have capital characters.
 ***********************************************************************/

int check_var_decl(vpiHandle argHandle) {


  char delim[] = "_";
  char *tok;
  int  count=0,i,line_no;

  char *tmp_str_buf;
  char *name,*name1,*ivl_file_name;
  //char *name;
  char c,msg[80];
  /* Get the line no of decalration */
  line_no = vpi_get(vpiLineNo,argHandle);
  /* Get the file where variable is decalred */
  tmp_str_buf = vpi_get_str(vpiFile,argHandle); 
  ivl_file_name = strdup (tmp_str_buf);
  // free (tmp_str_buf);

  /* Get string name of the reg */
  tmp_str_buf = vpi_get_str(vpiName,argHandle); 
  name = strdup (tmp_str_buf);
  // free (tmp_str_buf);

  /* check if length is more than 20 */ 
  if ( strlen(name) > 20) {    
    vpi_printf("ERROR :variable Declaration with more than 20 characters found  \'%s\' .\n           File : %s, line no : %d\n",
	       name, ivl_file_name,line_no); 
  }                                                       
  /* check if length is less than 3 */
  else if ( strlen(name) < 3) { 
    vpi_printf("ERROR :variable Declaration with less than 3 characters found \'%s\' .\n             File : %s, line no : %d\n\n",
	       name,ivl_file_name,line_no); 
  }

  /* count the no of underscores. */
  else    {
    name1=malloc(strlen(name));
    strcpy(name1,name);
    tok = strtok(name1,delim);
    while( ( tok != NULL))
      {
	tok = strtok( NULL, delim );
	count++;
      }
    if(count >3) {
      vpi_printf("ERROR :variable Declaration with more than 3 underscores found \'%s\' .\n              File : %s, line no : %d\n\n",
		 name,ivl_file_name,line_no);
    }
    else {
      for(i=0;i< strlen(name);i++)
	{
	  if(isupper(name[i]))
	    {
	      vpi_printf("ERROR :variable Declaration with capital letter(s)= \'%s\' .\n           File : %s, line no : %d\n\n",
			 name,ivl_file_name,line_no);
	      break;
	    }
	}  // for loop.
    } //else
  }
       
}



/***********************************************************************
 * check_mod_decl():
 * Identifies all module declarations in the design with -
 *         Names that are longer than 20 characters.
 *         Names that have more than 4 "_" in them.
 *         Names that have capital characters.
 *         Instance names which dont have module name in them.
 ***********************************************************************/



int check_mod_decl(vpiHandle argHandle) {
	char *ivl_mod_name;
	char *ivl_inst_name;
	char *ivl_full_name;
  char *ivl_file_name;
  char *lw_name,*rev_name;
  char *tmp_str_buf;

  char c,msg[80],c1;
  int  count=0,i,len1,len,line_no;

  vpiHandle moduleHandle ;


  vpi_printf ("IVL_UVM: check_mod_decl\n");

  if(vpi_get(vpiTopModule,argHandle) ) {
    return(0) ; // it should be a test bench.
  }
  /* Get the line no of decalration */
  line_no = vpi_get(vpiLineNo,argHandle);
  /* Get the file where module is decalred */
  tmp_str_buf = vpi_get_str(vpiFile,argHandle); 
  ivl_file_name = strdup (tmp_str_buf);
  // free (tmp_str_buf);

  /* Get string name of the instance */
  tmp_str_buf = vpi_get_str(vpiName,argHandle); 
  ivl_inst_name = strdup (tmp_str_buf);
  // free (tmp_str_buf);



  if ( strlen(ivl_inst_name) > 20) {
    vpi_printf("ERROR :Instance Declaration with more than 20 characters found \'%s\'. \n            File : %s, line no : %d\n",
	       ivl_inst_name,ivl_file_name,line_no); 
  }
  if ( strlen(ivl_inst_name) < 3) {
    vpi_printf("ERROR :Instance Declaration with less than 3 characters found \'%s\' \n              File : %s, line no : %d\n",
	       ivl_inst_name,ivl_file_name,line_no); 
  }

  tmp_str_buf = vpi_get_str(vpiDefName,argHandle);
  ivl_mod_name = strdup (tmp_str_buf);
  // free (tmp_str_buf);

       
  // to see if the instance name starts with u*
  c = ivl_inst_name[0];
  c1= 'u';

  tmp_str_buf = vpi_get_str(vpiDefName,argHandle);
  ivl_mod_name = strdup (tmp_str_buf);
  tmp_str_buf = vpi_get_str(vpiName,argHandle);
  ivl_inst_name = strdup (tmp_str_buf);
  tmp_str_buf = vpi_get_str(vpiFullName,argHandle);
  ivl_full_name = strdup (tmp_str_buf);

#ifndef IVL_UVM_NO_DEBUG
  vpi_printf ("IVL_UVM: module: %0s instance: %0s FullName: %0s \n",
			  ivl_mod_name,	
			  ivl_inst_name,	
			  ivl_full_name	
	      );

  // vpi_printf ("IVL_UVM: name: %0s name[0]: %c c: %c \n",
  //		 name, name[0], c);

#endif // IVL_UVM_DEBUG
       
  // IVL_UVM if(c != c1)
  if(ivl_inst_name[0] != c1)
    { vpi_printf("ERROR :instance Declaration doesnt start with u*_ \'%s\'  \n           File : %s, line no : %d\n",
		 ivl_inst_name,ivl_file_name,line_no);
    }
  len = strlen(ivl_inst_name);
  len1 = strlen(ivl_mod_name);

  /* To check if instance name is u*_module_name */
  for ( i = 0 ; i < strlen(ivl_mod_name) ; i++)
    {
      if(ivl_mod_name[len1-i] != ivl_inst_name[len-i]) {
	      vpi_printf("ERROR : Instance name \'%s\' doesnt contain module name \'%s\'  \n            File : %s, line no : %d\n",
		       ivl_inst_name,ivl_mod_name,ivl_file_name,line_no);
	      break;
	    }
    }

	return 0;
} // check_mod_decl



int check_name (vpiHandle top_module_h){
 
  vpiHandle module_h,module_i,variable_h,variable_i;
 

  module_i = vpi_iterate( vpiModule,top_module_h);
  if(module_i != NULL) {
    while( (module_h=vpi_scan(module_i)) != NULL) {
      check_name(module_h); // call recurcively till u reach the bottom.
    }
  }


  // check module name...
  check_mod_decl(top_module_h);
  
  variable_i = vpi_iterate (vpiVariables,top_module_h);
  if(variable_i != NULL) { 
    while( (variable_h =vpi_scan(variable_i)) != NULL) {
      check_var_decl(variable_h);
    }
  }                            
  variable_i = vpi_iterate (vpiNet,top_module_h);
  if(variable_i != NULL) {
    while( (variable_h =vpi_scan(variable_i)) != NULL) {
      check_var_decl(variable_h);
    }
  }
  variable_i = vpi_iterate (vpiReg,top_module_h);
  if(variable_i != NULL) {
    while( (variable_h =vpi_scan(variable_i)) != NULL) {
      check_var_decl(variable_h);
    }
  }
  return(0);
}


/**********************************************************************
 * Top routine  
 *********************************************************************/


int check_name_top() {
  vpiHandle module_h,module_it;

  module_it = vpi_iterate(vpiModule,NULL);
  while((module_h= vpi_scan(module_it)) != NULL) {
    check_name(module_h);
  }
  return(0);

}





