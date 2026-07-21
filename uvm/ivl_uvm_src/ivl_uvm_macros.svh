// ========== Copyright Header Begin ==========================
// 
// Project: IVL_UVM
// File: ivl_uvm_macros.svh
// Author(s): Srinivasan Venkataramanan 
//
// Copyright (c) VerifWorks 2016-2020  All Rights Reserved.
// Contact us via: support@verifworks.com
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES.
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License version 3 as published by the Free Software Foundation.
// 
// This program is distributed in the hope that it will be 
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with this work; if not, write to the Free Software
// 
// ========== Copyright Header End ============================
////////////////////////////////////////////////////////////////////////
`ifndef IVL_UVM_MACROS
  `define IVL_UVM_MACROS

 `define g2u_display(MSG, VERBOSITY=UVM_MEDIUM) \
   begin \
     if (uvm_report_enabled(VERBOSITY,UVM_INFO,get_name())) begin \
       uvm_report_info (get_name(), MSG, VERBOSITY, `uvm_file, `uvm_line); \
       uvm_count_info(); \
     end \
   end

  `define g2u_rand(obj, CNST = {} ) \
    begin \
      int rval; \
      rval = obj.randomize() with CNST ; \
      if (!rval) \
        $error ("Randomization failed"); \
    end 
  
  
  `define g2u_printf(FORMAT_MSG,VERBOSITY=UVM_MEDIUM) \
     begin \
       if (uvm_report_enabled(VERBOSITY,UVM_INFO,get_name())) \
         uvm_report_info (get_name(), $sformatf FORMAT_MSG, VERBOSITY, `uvm_file, `uvm_line); \
     end
  
  
  `define GO2UVM_DISP_ARG(arg) `"arg`"
  
  `define IVL_UVM_VPA $value$plusargs
   
  `ifndef UVM_TESTNAME
    `define UVM_TESTNAME ivl_uvm_default_test
  `endif // UVM_TESTNAME
  

  `ifndef IVL_UVM_MBX_T
    `define IVL_UVM_MBX_T int
  `else
    typedef class `IVL_UVM_MBX_T;
  `endif // IVL_UVM_MBX_T

`endif //  IVL_UVM_MACROS

`include "uvm_macros.svh"

