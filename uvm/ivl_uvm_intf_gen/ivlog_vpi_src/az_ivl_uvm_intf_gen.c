#include <stdlib.h>    /* ANSI C standard library */
#include <stdio.h>     /* ANSI C standard input/output library */
#include <stdarg.h>    /* ANSI C standard arguments library */
#include "vpi_user.h"  /* IEEE 1364 PLI VPI routine library  */
#include <string.h>    /* ANSI C string library */
#include <time.h>    /* ANSI C string library */

#include "az_ivl_uvm_common.h"

/* prototypes of routines in this PLI application */
PLI_INT32  IVL_UVM_IGEN_compiletf();
PLI_INT32  IVL_UVM_IGEN_calltf();

/**********************************************************************
 * VPI Registration Data
 *********************************************************************/
void IVL_UVM_IGEN_register()
{
  s_vpi_systf_data tf_data;
  tf_data.type        = vpiSysTask;
  tf_data.sysfunctype = 0;
  tf_data.tfname      = "$ivl_uvm_intf_gen";
  tf_data.calltf      = IVL_UVM_IGEN_calltf;
  tf_data.compiletf   = IVL_UVM_IGEN_compiletf;
  tf_data.sizetf      = NULL;
  tf_data.user_data   = NULL;

  vpi_register_systf(&tf_data);
}
/*********************************************************************/


/**********************************************************************
 * Compiletf application
 *********************************************************************/
PLI_INT32 IVL_UVM_IGEN_compiletf() 
{
  vpiHandle systf_h, tfarg_itr;

  systf_h = vpi_handle(vpiSysTfCall, NULL);
  if (systf_h == NULL) {
    vpi_printf("ERROR: ivl_uvm_intf_gen could not obtain handle to systf call\n");
    vpi_control(vpiFinish, 1);  /* abort simulation */
    return(0);
  }
  tfarg_itr = vpi_iterate(vpiArgument, systf_h);
  if (systf_h == NULL) {
    vpi_printf("ERROR: ivl_uvm_intf_gen could not obtain iterator to systf args\n");
    vpi_control(vpiFinish, 1);  /* abort simulation */
    return(0);
  }
    return(0);
}

/**********************************************************************
 * calltf routine
 *********************************************************************/
PLI_INT32 IVL_UVM_IGEN_calltf()
{
  vpiHandle systf_h, mod_h,
            port_itr, port_h;

  FILE * ivl_uvm_ifp;
  FILE * ivl_uvm_tbp;
  char * ivl_uvm_mod_name;
  char * ivl_uvm_mod_name_saved;
  char * ivl_uvm_port_name;
  char * ivl_uvm_clk_name;
  char ivl_uvm_dut_name [100];
  char ivl_uvm_intf_name [100];
  char ivl_uvm_intf_inst_name [100];
  char ivl_uvm_intf_fname [100];
  char ivl_uvm_tb_name [100];
  char ivl_uvm_tb_fname [100];
  vpiHandle ivl_uvm_top_mod_itr;
  PLI_INT32 ivl_uvm_port_size;
  PLI_INT32 ivl_uvm_port_count;
  PLI_INT32 ivl_uvm_mp_c;
  PLI_INT32 ivl_uvm_tb_sig_c;
  PLI_INT32 clk_cmp_val;
  char * ivl_uvm_mp_delimiter;
  

  
  ivl_uvm_pr_copyright ();


  /* get module handle from first system task argument.  Assume the  */
  /* compiletf routine has already verified correct argument type.   */
  systf_h = vpi_handle(vpiSysTfCall, NULL);
  if (systf_h == NULL) {
    vpi_printf("ERROR: ivl_uvm_intf_gen could not obtain handle to systf call\n");
    return(0);
  }
  ivl_uvm_top_mod_itr = vpi_iterate(vpiModule, NULL);
  if (systf_h == NULL) {
    vpi_printf("ERROR: ivl_uvm_intf_gen could not obtain iterator to systf args\n");
    return(0);
  }
  mod_h = vpi_scan(ivl_uvm_top_mod_itr);
  vpi_free_object(ivl_uvm_top_mod_itr);  /* free itr since did not scan until nul */

  ivl_uvm_mod_name = vpi_get_str(vpiDefName, mod_h);
  ivl_uvm_mod_name_saved = vpi_get_str(vpiDefName, mod_h);
  vpi_printf("\nGenerating SystemVerilog Interface for Module \"%s\" \n", ivl_uvm_mod_name);
  strcpy (ivl_uvm_dut_name, ivl_uvm_mod_name);
  strcpy (ivl_uvm_intf_name, ivl_uvm_mod_name);
  strcpy (ivl_uvm_intf_fname, ivl_uvm_mod_name);
  strcpy (ivl_uvm_tb_name, ivl_uvm_mod_name);
  strcpy (ivl_uvm_tb_fname, ivl_uvm_mod_name);
  strcat (ivl_uvm_intf_name, "_if");
  strcpy (ivl_uvm_intf_inst_name, ivl_uvm_intf_name);
  strcat (ivl_uvm_intf_inst_name, "_0");
  strcat (ivl_uvm_intf_fname, "_if.sv");
  strcat (ivl_uvm_tb_name, "_tb");
  strcat (ivl_uvm_tb_fname, "_tb.sv");

  ivl_uvm_ifp = fopen (ivl_uvm_intf_fname, "w");
  fprintf(ivl_uvm_ifp, "// Automatically generated from IVL_UVM's Interface Generator utility \n");

  fprintf(ivl_uvm_ifp, "// Using IVL_UVM_CLK as a text macro for clock \n");
  fprintf(ivl_uvm_ifp, "// If your clock is other than \"clk\" change the line below\n");
  fprintf(ivl_uvm_ifp, "`define IVL_UVM_CLK clk \n");
  fprintf(ivl_uvm_ifp, "interface %s (input logic `IVL_UVM_CLK);\n", ivl_uvm_intf_name);

  port_itr = vpi_iterate(vpiPort, mod_h);
    if (!port_itr) {
      vpi_printf("   No ports found\n");
      return(0);
    }

  ivl_uvm_port_count = 0;

  while ((port_h = vpi_scan(port_itr))) {
    ivl_uvm_port_count++;
    ivl_uvm_port_name = vpi_get_str(vpiName, port_h);
    ivl_uvm_clk_name = "clk";
    clk_cmp_val = strncmp (ivl_uvm_port_name , ivl_uvm_clk_name, 3);
    if (clk_cmp_val == 0) continue;

    // vpi_printf("  Port name is %s clk_cmp_val: %d \n", ivl_uvm_port_name, clk_cmp_val);
    ivl_uvm_port_size = vpi_get(vpiSize, port_h);
    // vpi_printf("    Size is %d\n", ivl_uvm_port_size);
  
    fprintf(ivl_uvm_ifp, "  logic [%d : 0] %s;\n",
            ivl_uvm_port_size - 1,
            ivl_uvm_port_name);

    }


  // For clocking block
  //
  fprintf(ivl_uvm_ifp, "\n\n  `ifdef IVL_UVM_ALLOW_CB_IN_IF \n");
  fprintf(ivl_uvm_ifp, "\n\n  // Clocking block defintion for the TB side \n\n");
  fprintf(ivl_uvm_ifp, "  // Assuption made is: there exists a clock named \"clk\"\n");
  fprintf(ivl_uvm_ifp, "  clocking cb @(posedge `IVL_UVM_CLK);\n");
  port_itr = vpi_iterate(vpiPort, mod_h);
    if (!port_itr) {
      vpi_printf("   No ports found\n");
      return(0);
    }
  while ((port_h = vpi_scan(port_itr))) {
    ivl_uvm_port_name = vpi_get_str(vpiName, port_h);
    // vpi_printf("  Port name is %s\n", ivl_uvm_port_name);
    ivl_uvm_port_size = vpi_get(vpiSize, port_h);
    // vpi_printf("    Size is %d\n", ivl_uvm_port_size);
  
     // Skip clk signal
    clk_cmp_val = strncmp (ivl_uvm_port_name , ivl_uvm_clk_name, 3);
    if (clk_cmp_val == 0) continue;

     switch (vpi_get(vpiDirection, port_h)) {
      case vpiInput:  
        fprintf(ivl_uvm_ifp, "    output %s;\n",
            ivl_uvm_port_name);
        break;
      case vpiOutput: 
        fprintf(ivl_uvm_ifp, "    input %s;\n",
            ivl_uvm_port_name);
        break;
      case vpiInout:  
        fprintf(ivl_uvm_ifp, "    inout %s;\n",
            ivl_uvm_port_name);
        break;
    }
  }
  fprintf(ivl_uvm_ifp, "  endclocking : cb \n");
  fprintf(ivl_uvm_ifp, "\n  `endif // IVL_UVM_ALLOW_CB_IN_IF \n");

  // For modport
  //
  fprintf(ivl_uvm_ifp, "\n\n  // Modport defintion for the DUT \n\n");
  fprintf(ivl_uvm_ifp, "  modport dut_mp (\n");
  port_itr = vpi_iterate(vpiPort, mod_h);
    if (!port_itr) {
      vpi_printf("   No ports found\n");
      return(0);
    }
  ivl_uvm_mp_c = 0;

  while ((port_h = vpi_scan(port_itr))) {
    ivl_uvm_port_name = vpi_get_str(vpiName, port_h);
    ivl_uvm_port_size = vpi_get(vpiSize, port_h);

    ivl_uvm_mp_delimiter = "";
    if (ivl_uvm_mp_c < ivl_uvm_port_count - 1) {
      ivl_uvm_mp_delimiter = ",";
    }
    //vpi_printf ("ivl_uvm_mp_c %d ivl_uvm_port_count %d ivl_uvm_mp_del: %s",
    //             ivl_uvm_mp_c, ivl_uvm_port_count, ivl_uvm_mp_delimiter);
    ivl_uvm_mp_c++;
  
     switch (vpi_get(vpiDirection, port_h)) {
      case vpiInput:  
        fprintf(ivl_uvm_ifp, "    input %s%s\n",
            ivl_uvm_port_name, ivl_uvm_mp_delimiter);
        break;
      case vpiOutput: 
        fprintf(ivl_uvm_ifp, "    output %s%s\n",
            ivl_uvm_port_name, ivl_uvm_mp_delimiter);
        break;
      case vpiInout:  
        fprintf(ivl_uvm_ifp, "    inout %s%s\n",
            ivl_uvm_port_name, ivl_uvm_mp_delimiter);
        break;
    }
  }
  fprintf(ivl_uvm_ifp, "  ); // end of modport dut_mp \n");


  fprintf(ivl_uvm_ifp, "endinterface : %s \n", ivl_uvm_intf_name);
  vpi_printf("See file: \"%s\" for output\n",ivl_uvm_intf_fname);
  // For TB generation
  ivl_uvm_tbp = fopen (ivl_uvm_tb_fname, "w");
  fprintf(ivl_uvm_tbp, "// Automatically generated from IVL_UVM's Interface Generator utility \n");
  fprintf(ivl_uvm_tbp, "module %s; \n", ivl_uvm_tb_name);
  fprintf(ivl_uvm_tbp, "  timeunit 1ns; \n");
  fprintf(ivl_uvm_tbp, "  timeprecision 1ns; \n");
  fprintf(ivl_uvm_tbp, "  parameter IVL_UVM_CLK_PERIOD = 10; \n\n");

  fprintf(ivl_uvm_tbp, "  // Simple clock generator \n");
  fprintf(ivl_uvm_tbp, "  bit `IVL_UVM_CLK ;\n");
  fprintf(ivl_uvm_tbp, "  always # (IVL_UVM_CLK_PERIOD/2) `IVL_UVM_CLK <= ~`IVL_UVM_CLK;\n");

  fprintf(ivl_uvm_tbp, "\n  // Interface instance \n");
  fprintf(ivl_uvm_tbp, "  %s %s (.*); \n", ivl_uvm_intf_name, ivl_uvm_intf_inst_name);
  fprintf(ivl_uvm_tbp, "\n  // Connect TB clk to Interface instance clk \n");

  fprintf(ivl_uvm_tbp, "\n  // DUT instance \n");
  fprintf(ivl_uvm_tbp, "  %s %s_0 ( \n", ivl_uvm_dut_name, ivl_uvm_dut_name);

  port_itr = vpi_iterate(vpiPort, mod_h);
  if (!port_itr) {
    vpi_printf("   No ports found\n");
    return(0);
  }

  ivl_uvm_tb_sig_c = 0;
  while ((port_h = vpi_scan(port_itr))) {
    ivl_uvm_port_name = vpi_get_str(vpiName, port_h);
    ivl_uvm_mp_delimiter = "";
    if (ivl_uvm_tb_sig_c < ivl_uvm_port_count - 1) {
      ivl_uvm_mp_delimiter = ",";
    }
    ivl_uvm_tb_sig_c++;

    fprintf(ivl_uvm_tbp, "    .%s(%s.%s)%s \n", ivl_uvm_port_name, ivl_uvm_intf_inst_name, ivl_uvm_port_name, ivl_uvm_mp_delimiter);
  }
 
  fprintf(ivl_uvm_tbp, "  ); // %s DUT \n", ivl_uvm_dut_name);

  fprintf(ivl_uvm_tbp,  "\n  // Simple stimuli block, should use IVL_UVM_Go2UVM ideally  \n");
  fprintf(ivl_uvm_tbp,  "  initial begin : stim \n");
  fprintf(ivl_uvm_tbp,  "    // Example \n");
  fprintf(ivl_uvm_tbp,  "    // #10 %s.dut_input_signal <= 1'b1; \n", ivl_uvm_intf_inst_name);
  fprintf(ivl_uvm_tbp,  "    $display (\"Automatically generated Testbench from IVL_UVM's Interface Generator utility \"); \n");
  fprintf(ivl_uvm_tbp,  "    #100 $finish (2); \n");
  fprintf(ivl_uvm_tbp,  "  end : stim \n");
  
  fprintf(ivl_uvm_tbp, "endmodule : %s \n", ivl_uvm_tb_name);
  vpi_printf("See file: \"%s\" for output\n",ivl_uvm_tb_fname);
  vpi_printf("Include these 2 files in your next simulation for a quick TB: \"%s\" \"%s\" \n", ivl_uvm_intf_fname, ivl_uvm_tb_fname);

  return(0);
}
#ifndef VCS
/*********************************************************************/
void (*vlog_startup_routines[])() =
{
   IVL_UVM_IGEN_register, /* */
  0 /*** final entry must be 0 ***/

};
#endif
