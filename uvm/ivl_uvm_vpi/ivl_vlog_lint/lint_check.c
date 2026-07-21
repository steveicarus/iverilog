#define DBUG 0
#include <stdlib.h>    /* ANSI C standard library */
#include <stdio.h>     /* ANSI C standard input/output library */
#include <string.h>   // IVL_UVM added
#include "vpi_user.h"  /* IEEE 1364 PLI VPI routine library  */
#include "veriuser.h"  /* IEEE 1364 PLI TF routine library  */
                       /* using TF routines for simulation control */
/* prototypes of the PLI application routines */
// IVL_UVM fixed prototypes 
// int guru_lint_check_compiletf(), guru_lint_check_calltf();
int guru_lint_check_compiletf(char *user_data);
int guru_lint_check_calltf(char *user_data);


extern int check_name_top (void);

/**********************************************************************
 * $lint_check Registration Data
 * (add this function name to the vlog_startup_routines array)
 *********************************************************************/
void guru_lint_check_register(void)
{
  s_vpi_systf_data tf_data;

  tf_data.type        = vpiSysTask;
  tf_data.tfname      = "$lint_check";
  tf_data.calltf      = guru_lint_check_calltf;
  tf_data.compiletf   = guru_lint_check_compiletf;
  tf_data.sizetf      = NULL;
  vpi_register_systf(&tf_data);
  return;
}

int guru_lint_check_compiletf(char *user_data)
{
  vpiHandle   systf_handle, arg_iterator, arg_handle;
  s_vpi_value  constant_name;
  char *word;

  /* obtain a handle to the system task instance */
  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  if(systf_handle == NULL) {
    vpi_printf("SYNTAX ERROR : failed to get handle for systf \n");
    tf_dofinish();
    return(0);
  }
#if DBUG
  else {vpi_printf("Got handle to systf\n");}
#endif


  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if(arg_iterator == NULL) {
    vpi_printf("SYNTAX ERROR : $lint_check expects at least one argument\n");
    tf_dofinish();
    return(0);
  }
#if DBUG
  else{vpi_printf("Got arg_iterator\n"); }
#endif

  /*************** First argument processed here ****************************/
  arg_handle = vpi_scan(arg_iterator);
  if(arg_handle == NULL) { 
    vpi_printf("SYNTAX ERROR : $lint_check expects at least one argument\n");
    tf_dofinish();return(0);
  } // only one input was there...

  if(( vpi_get(vpiType,arg_handle)) != vpiConstant ) {
    vpi_printf("SYNTAX ERROR : No inputs given to  $lint_check \n"); 
    tf_dofinish();
    return(0);
  }

  constant_name.format = vpiStringVal;
  vpi_get_value(arg_handle,&constant_name);
  word = constant_name.value.str;

  if( strcmp(word,"check_name") == 0 ){
    vpi_printf("Found option =%s -%d\n",word,strcmp(word,"check_name"));
    vpi_printf("Starting the check for names ...\n");
    check_name_top(); 
  }
  else if(strcmp(word,"check_port") == 0) {
    vpi_printf("Error: Unsupported in Icarus  - check for ports ...\n");
    //IVL_UVM check_port();
  }
  else if(strcmp(word,"check_if") == 0) {
    vpi_printf("Error: Unsupported in Icarus  - check for if's ...\n");
    //IVL_UVM check_if();
  }
  else{
    vpi_printf("SYNTAX ERROR : $lint_check is given an invalid option1 - \"%s\"\n",word);
      
    tf_dofinish();
    return(0);
  }
  

  /*************** second argument processed here ****************************/
  arg_handle = vpi_scan(arg_iterator);
  if(arg_handle == NULL) { tf_dofinish();return(0);} // only one input was there...

  if(( vpi_get(vpiType,arg_handle)) != vpiConstant ) {
    vpi_printf("SYNTAX ERROR : Invalid option given to  $lint_check \n");
    tf_dofinish();
    return(0);
  }

  constant_name.format = vpiStringVal;
  vpi_get_value(arg_handle,&constant_name);
  word = constant_name.value.str;

  if(~strcmp(word,"check_name") ) {check_name_top();}
  else if(strcmp(word,"check_port") == 0) {
    vpi_printf("Error: Unsupported in Icarus  - check for ports ...\n");
    //IVL_UVM check_port();
  }
  else if(strcmp(word,"check_if") == 0) {
    vpi_printf("Error: Unsupported in Icarus  - check for if's ...\n");
    //IVL_UVM check_if();
  }
  else {
    vpi_printf("SYNTAX ERROR : $lint_check is given an invalid option2 - \"%s\"\n",word);
    tf_dofinish();
    return(0);
  }


  /*************** Third argument processed here ****************************/
  arg_handle = vpi_scan(arg_iterator);
  if(arg_handle == NULL) { tf_dofinish();return(0);} // only two inputs were there...


  if(( vpi_get(vpiType,arg_handle)) != vpiConstant ) {
    vpi_printf("SYNTAX ERROR : Invalid option given to  $lint_check \n");
    tf_dofinish();
    return(0);
  }

  constant_name.format = vpiStringVal;
  vpi_get_value(arg_handle,&constant_name);
  word = constant_name.value.str;

  if(~strcmp(word,"check_name") ){check_name_top();}
  else if(~strcmp(word,"check_port") ) {
    vpi_printf("Error: Unsupported in Icarus  - check for ports ...\n");
    //IVL_UVM check_port();
  }
  else if(~strcmp(word,"check_if")) {
    vpi_printf("Error: Unsupported in Icarus  - check for if's ...\n");
    //IVL_UVM check_if();
  }
  else {
    vpi_printf("SYNTAX ERROR : $lint_check is given an invalid option3 - \"%s\"\n",word);
    tf_dofinish();
    return(0);
  }

  /*************** Check for more arguments      ****************************/
  if(vpi_scan(arg_iterator) != NULL ) {
    vpi_printf("SYNTAX ERROR :  $lint_check is given more than 3 inputs\n");
  }

  tf_dofinish();
  return(0);


}


int guru_lint_check_calltf(char *user_data)
{

  return(0);
}

// IVL_UVM added
void (*vlog_startup_routines[])(void) =
  {
    guru_lint_check_register, /* */
    0 /*** final entry must be 0 ***/

  };
