// ========== Copyright Header Begin ==========================
// 
// Project: IVL_UVM
// File: ivl_uvm_comps.svh
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


class uvm_sequence_item extends uvm_object;
  int sequence_id;

  function new (string name = "uvm_sequence_item");
    super.new (name);
  endfunction : new 

endclass : uvm_sequence_item 

class uvm_report_object extends uvm_object;
  function new (string name = "uvm_report_object");
    super.new (name);
  endfunction : new 

endclass : uvm_report_object 

class uvm_phase extends uvm_object;
  function new (string name = "uvm_phase");
    super.new (name);
  endfunction : new 

endclass : uvm_phase 

virtual class uvm_component extends uvm_report_object;
  function new (string name = "uvm_component", uvm_component parent);
    super.new (name);
  endfunction : new 

  virtual function void build_phase(uvm_phase phase);
    `g2u_display ("build_phase", UVM_HIGH)
  endfunction : build_phase

  virtual function void connect_phase(uvm_phase phase);
    `g2u_display ("connect_phase", UVM_HIGH)
  endfunction : connect_phase

  virtual function void end_of_elaboration_phase(uvm_phase phase);
    `g2u_display ("end_of_elaboration_phase", UVM_HIGH)
  endfunction : end_of_elaboration_phase
  virtual function void start_of_simulation_phase(uvm_phase phase);
    `g2u_display ("start_of_simulation_phase", UVM_HIGH)
  endfunction : start_of_simulation_phase

  virtual task run_phase (uvm_phase phase);
    `g2u_display ("run_phase", UVM_HIGH)
    this.print (uvm_default_printer);
  endtask : run_phase 

  virtual function void extract_phase(uvm_phase phase);
  endfunction : extract_phase
  virtual function void check_phase(uvm_phase phase);
  endfunction : check_phase
  virtual function void report_phase(uvm_phase phase);
  endfunction : report_phase
  virtual function void final_phase(uvm_phase phase);
  endfunction : final_phase

  virtual task ivl_uvm_run_all_phases ();
    uvm_phase u_ph_0;

    u_ph_0 = new();
    this.build_phase (u_ph_0);
    this.connect_phase (u_ph_0);
    this.end_of_elaboration_phase (u_ph_0);
    this.start_of_simulation_phase (u_ph_0);
    this.run_phase (u_ph_0);
    this.extract_phase (u_ph_0);
    this.check_phase (u_ph_0);
    this.report_phase (u_ph_0);
    this.final_phase (u_ph_0);
  endtask : ivl_uvm_run_all_phases 

endclass : uvm_component

virtual class uvm_test extends uvm_component;
  function new (string name = "uvm_test", uvm_component parent = null);
    super.new (name, parent);
    `g2u_display ("%m");
  endfunction : new 
endclass : uvm_test 

// Default concrete test so run_test() can instantiate when no +define+UVM_TESTNAME=... is given.
// (Icarus elaboration rejects new() on a virtual class type.)
class ivl_uvm_default_test extends uvm_test;
  function new (string name = "ivl_uvm_default_test", uvm_component parent = null);
    super.new(name, parent);
  endfunction
endclass : ivl_uvm_default_test


