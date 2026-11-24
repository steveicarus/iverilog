// event control and wait inside class tasks, handle switched between waits
interface bus_if;
  logic       clk;
  logic       ready;
  logic [7:0] data;
endinterface

class driver;
  virtual bus_if vif;
  task run(int n);
    repeat (n) begin
      @(posedge vif.clk);
      vif.data <= vif.data + 1;
    end
  endtask
  task wait_ready();
    wait (vif.ready);
  endtask
endclass

module test;
  bus_if b1();
  bus_if b2();
  driver d;
  bit pass = 1;

  initial begin
    b1.clk = 0; b2.clk = 0;
    forever #5 begin b1.clk = ~b1.clk; b2.clk = ~b2.clk; end
  end

  initial begin
    b1.data = 0; b2.data = 8'h10; b1.ready = 0; b2.ready = 0;
    d = new;
    d.vif = b1;
    d.run(3);
    #1;
    if (b1.data !== 3 || b2.data !== 8'h10) begin $display("FAILED: run b1 %h %h", b1.data, b2.data); pass = 0; end
    d.vif = b2;
    d.run(2);
    #1;
    if (b1.data !== 3 || b2.data !== 8'h12) begin $display("FAILED: run b2 %h %h", b1.data, b2.data); pass = 0; end
    fork
      d.wait_ready();
      #100 begin $display("FAILED: wait_ready timeout"); pass = 0; end
    join_any
    if ($time != 70) begin $display("FAILED: ready at %0t", $time); pass = 0; end
    if (pass) $display("PASSED");
    $finish;
  end

  initial begin
    #60 b1.ready = 1;  // the handle is on b2: must not end the wait
    #10 b2.ready = 1;
  end
endmodule
