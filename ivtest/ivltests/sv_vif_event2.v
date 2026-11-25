// event control through a handle that changes at run time
interface bus_if;
  logic [7:0] data;
  logic       clk;
endinterface

module test;
  bus_if b1();
  bus_if b2();
  virtual bus_if v;
  integer n_pos = 0;
  integer n_any = 0;
  bit pass = 1;

  initial begin
    b1.clk = 0; b2.clk = 0;
    forever #5 b1.clk = ~b1.clk;
  end
  initial begin
    #1;
    forever #3 b2.clk = ~b2.clk;
  end

  initial begin
    v = b1;
    @(posedge v.clk);
    if ($time != 5) begin $display("FAILED: b1 posedge at %0t", $time); pass = 0; end
    v = b2;
    @(posedge v.clk);
    if ($time != 10) begin $display("FAILED: b2 posedge at %0t", $time); pass = 0; end
    @(negedge v.clk);
    if ($time != 13) begin $display("FAILED: b2 negedge at %0t", $time); pass = 0; end
    v = b1;
    @(v.data);
    if ($time != 30) begin $display("FAILED: b1 data at %0t", $time); pass = 0; end
    b1.data = 0;
    #1;
    fork
      wait (v.data == 8'h7);
      #100 begin $display("FAILED: wait timeout"); pass = 0; end
    join_any
    if ($time != 40) begin $display("FAILED: wait at %0t", $time); pass = 0; end
    if (pass) $display("PASSED");
    $finish;
  end

  initial begin
    #25 b2.data = 1;  // must not wake the @(v.data) above
    #5 b1.data = 1;
    #5 b2.data = 7;   // must not end the wait
    #5 b1.data = 7;
  end
endmodule
