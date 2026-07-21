// ========== Copyright Header Begin ==========================
// 
// Project: IVL_UVM
// File: ivl_uvm_patches.svh
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
`ifndef __IVL_UVM_PATCHES__
`define __IVL_UVM_PATCHES__

  `define IVL_UVM_I419
  // Function: uvm_report_enabled
  //
  // Returns 1 if the configured verbosity for this severity/id is greater than 
  // ~verbosity~ and the action associated with the given ~severity~ and ~id~
  // is not UVM_NO_ACTION, else returns 0.
  // 
  // See also <get_report_verbosity_level> and <get_report_action>, and the
  // global version of <uvm_report_enabled>.


  function int uvm_report_enabled(int verbosity, 
                          uvm_severity severity=UVM_INFO, string id="");
    if (get_report_verbosity_level(severity, id) < verbosity) begin
        // get_report_action(severity,id) == uvm_action'(UVM_NO_ACTION))  */
      return 0;
    end else begin
      return 1;
    end
  endfunction : uvm_report_enabled

  function void m_check_verbosity();
    string verb_string;
    int verb_count;
    int plusarg;
    int verbosity;

    verbosity = 10;

    verb_count = $value$plusargs("UVM_VERBOSITY=%s", verb_string);
    
   
    verbosity = int'(UVM_MEDIUM);

    if (verb_count) begin

      if (verb_string == "UVM_NONE" || verb_string == "NONE") begin
        verbosity = int'(UVM_NONE);
      end

      if (verb_string == "UVM_LOW" || verb_string == "LOW") begin
        verbosity = int'(UVM_LOW);
      end

      if (verb_string == "UVM_MEDIUM" || verb_string == "MEDIUM") begin
        verbosity = int'(UVM_MEDIUM);
      end

      if (verb_string == "UVM_HIGH" || verb_string == "HIGH") begin
        verbosity = int'(UVM_HIGH);
      end

      if (verb_string == "UVM_FULL" || verb_string == "FULL") begin
        verbosity = int'(UVM_FULL);
      end

      if (verb_string == "UVM_DEBUG" || verb_string == "DEBUG") begin
        verbosity = int'(UVM_DEBUG);
      end



  /*
      case(verb_string)
        "UVM_NONE"    : verbosity = UVM_NONE;
        "NONE"        : verbosity = UVM_NONE;
        "UVM_LOW"     : verbosity = UVM_LOW;
        "LOW"         : verbosity = UVM_LOW;
        "UVM_MEDIUM"  : verbosity = UVM_MEDIUM;
        "MEDIUM"      : verbosity = UVM_MEDIUM;
        "UVM_HIGH"    : verbosity = UVM_HIGH;
        "HIGH"        : verbosity = UVM_HIGH;
        "UVM_FULL"    : verbosity = UVM_FULL;
        "FULL"        : verbosity = UVM_FULL;
        "UVM_DEBUG"   : verbosity = UVM_DEBUG;
        "DEBUG"       : verbosity = UVM_DEBUG;
        default       : verbosity = UVM_MEDIUM;
      endcase
  */
    end

    ivl_uvm_glb_verb = verbosity;
  endfunction : m_check_verbosity

  // Function: get_report_verbosity_level
  //
  // Gets the verbosity level in effect for this object. Reports issued
  // with verbosity greater than this will be filtered out. The severity
  // and tag arguments check if the verbosity level has been modified for
  // specific severity/tag combinations.

  function int get_report_verbosity_level(uvm_severity severity=UVM_INFO, string id="");
    m_check_verbosity ();
    return ivl_uvm_glb_verb;
  endfunction : get_report_verbosity_level


  function string get_name ();
    return "IVL_GO2UVM";
  endfunction : get_name 

  typedef class uvm_report_object;

  class uvm_object_wrapper;
  endclass : uvm_object_wrapper
  
  class uvm_objection;
  endclass
  class uvm_printer;
  endclass
  class uvm_recorder;
  endclass
  class uvm_comparer;
  endclass
  class uvm_packer;
  endclass
  class uvm_copy_map;
  endclass
  
  // typedef bit uvm_bitstream_t;
  
  class uvm_status_container;
  endclass : uvm_status_container

`define IVL_UVM_REF 

`endif // __IVL_UVM_PATCHES__

