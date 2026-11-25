// Check that event controls on a member of a virtual interface wait for the
// instance that the handle refers to when the wait starts, also when the
// handle is changed between waits.

interface bus_if;
  logic       clk = 1'b0;
  logic [7:0] data = 8'h00;
endinterface

module test;

  bus_if a();
  bus_if b();

  virtual bus_if vif = a;
  virtual bus_if mon = a;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", (exp), (val)); \
      failed = 1'b1; \
    end \
  while (0)

  class waiter;
    virtual bus_if vif;
    int count = 0;

    task run();
      forever begin
        @(negedge vif.clk);
        count += 1;
      end
    endtask
  endclass

  waiter w;

  // a.clk toggles every 5, b.clk toggles every 20
  always #5 a.clk = ~a.clk;
  always #20 b.clk = ~b.clk;

  int pos_count = 0;
  int any_count = 0;
  time last_pos = 0;

  always @(posedge vif.clk) begin
    pos_count += 1;
    last_pos = $time;
  end

  always @(mon.data) any_count += 1;

  initial begin
    w = new;
    w.vif = b;
    fork
      w.run();
    join_none

    // Rising edges of a.clk at 5, 15, 25, 35
    #38;
    `check(pos_count, 4);
    `check(last_pos, 35);
    // Falling edges of b.clk at 40, 80, 120, ...
    `check(w.count, 0);

    // The pending wait is for a.clk (45). After that the waits are for
    // b.clk: 60, 100
    vif = b;
    #70;
    `check(pos_count, 7);
    `check(last_pos, 100);
    `check(w.count, 2);

    // Changes of the value in the other instance are not events
    b.data = 8'h01;
    #1;
    `check(any_count, 0);
    a.data = 8'h01;
    #1;
    `check(any_count, 1);
    mon.data = 8'h02;
    #1;
    `check(any_count, 2);

    // The next wait starts after this, so it is a wait for b.data
    mon = b;
    a.data = 8'h03;
    #1;
    `check(any_count, 3);
    a.data = 8'h04;
    #1;
    `check(any_count, 3);
    b.data = 8'h05;
    #1;
    `check(any_count, 4);

    // Wait in line for both instances
    vif = a;
    @(posedge vif.clk);
    `check(a.clk, 1'b1);
    `check($time % 10, 5);
    vif = b;
    @(negedge vif.clk);
    `check(b.clk, 1'b0);
    `check($time % 40, 0);
    @(vif.clk);
    `check(b.clk, 1'b1);
    `check($time % 40, 20);

    if (!failed) begin
      $display("PASSED");
    end
    $finish(0);
  end

endmodule
