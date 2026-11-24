// Check that event controls and wait statements on members selected through a
// virtual interface use the selected instance. An event control reads the
// handle when it starts, so a wait that is pending when the handle is
// assigned still waits on the old instance.

interface bus_if;
  logic clk = 0;
  logic [7:0] data = 0;
  logic rst_n = 1;
endinterface

class mon;
  virtual bus_if vif;
  int count = 0;
  function new(virtual bus_if v); vif = v; endfunction
  task run();
    forever begin
      @(posedge vif.clk);
      count++;
    end
  endtask
  task wait_data();
    @(vif.data);
  endtask
  task wait_either();
    @(negedge vif.rst_n or posedge vif.clk);
  endtask
endclass

module test;
  bus_if b0();
  bus_if b1();
  virtual bus_if v;
  mon m;
  int fails = 0;
  int n_static = 0;
  bit data_seen = 0, either_seen = 0;

  // The handle is null until the first initial block sets it.
  initial #1 forever @(posedge v.clk) n_static++;

  initial begin
    v = b0;
    m = new(b0);
    fork
      m.run();
    join_none
    #1;
    // b1 toggles: nobody listens
    repeat (3) begin #1 b1.clk = 1; #1 b1.clk = 0; end
    if (m.count != 0 || n_static != 0) begin $display("FAILED: wrong instance %0d %0d", m.count, n_static); fails++; end
    repeat (2) begin #1 b0.clk = 1; #1 b0.clk = 0; end
    if (m.count != 2 || n_static != 2) begin $display("FAILED: count %0d %0d", m.count, n_static); fails++; end
    // rebind while waiting: the pending waits are on b0
    m.vif = b1;
    v = b1;
    #1 b1.clk = 1; #1 b1.clk = 0;
    if (m.count != 2 || n_static != 2) begin $display("FAILED: pending wait moved %0d %0d", m.count, n_static); fails++; end
    #1 b0.clk = 1; #1 b0.clk = 0;
    if (m.count != 3 || n_static != 3) begin $display("FAILED: pending wait %0d %0d", m.count, n_static); fails++; end
    // the next waits are on b1
    repeat (2) begin #1 b1.clk = 1; #1 b1.clk = 0; end
    if (m.count != 5 || n_static != 5) begin $display("FAILED: missed wake-up %0d %0d", m.count, n_static); fails++; end
    repeat (3) begin #1 b0.clk = 1; #1 b0.clk = 0; end
    if (m.count != 5 || n_static != 5) begin $display("FAILED: stale handle %0d %0d", m.count, n_static); fails++; end
    // negedge must not count
    #1 b1.clk = 1;
    #1 if (m.count != 6) begin $display("FAILED: posedge"); fails++; end
    #1 b1.clk = 0;
    #1 if (m.count != 6) begin $display("FAILED: negedge counted"); fails++; end
    fork
      begin m.wait_data(); data_seen = 1; end
      begin m.wait_either(); either_seen = 1; end
    join_none
    #1 b0.data = 8'h12; b0.rst_n = 0;
    #1 if (data_seen || either_seen) begin $display("FAILED: wrong instance data"); fails++; end
    #1 b1.data = 8'h01;
    #1 if (!data_seen || either_seen) begin $display("FAILED: data event"); fails++; end
    #1 b1.rst_n = 0;
    #1 if (!either_seen) begin $display("FAILED: list event"); fails++; end
    wait (v.data == 8'h01);
    fork
      #5 b1.data = 8'h77;
    join_none
    wait (v.data == 8'h77);
    if (b1.data !== 8'h77) begin $display("FAILED: wait"); fails++; end
    if (fails == 0) $display("PASSED");
    $finish;
  end
endmodule
