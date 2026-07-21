virtual class uvm_void;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_void

class uvm_object extends uvm_void;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_object

class uvm_transaction extends uvm_object;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_transaction

class uvm_sequence_item extends uvm_transaction;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_sequence_item

class uvm_sequence_base extends uvm_sequence_item;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_sequence_base

class uvm_report_object extends uvm_object;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_report_object

class uvm_component extends uvm_report_object;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_component

class uvm_subscriber extends uvm_component;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_subscriber

class uvm_env extends uvm_component;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_env

class uvm_test extends uvm_component;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_test

class uvm_driver extends uvm_component;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_driver

class uvm_monitor extends uvm_component;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_monitor

class uvm_scoreboard extends uvm_component;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_scoreboard

class uvm_agent extends uvm_component;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_agent

class uvm_sequencer extends uvm_component;
  function new();
    $display("%m");
  endfunction : new
endclass : uvm_sequencer

module top;
  uvm_component comp0;
  initial begin : test_uvm
    comp0 = new();
    $display ("Booting up IVL_UVM ...");
  end : test_uvm
  uvm_sequence_base seq0;
  initial begin : seq_uvm
    seq0 = new();
    $display ("Booting up Class Hierachy ...");
  end : seq_uvm
  uvm_sequencer sequencer0;
  initial begin : sequencer_uvm
    sequencer0 = new();
    $display ("Booting up uvm_component ...");
  end : sequencer_uvm
endmodule : top
