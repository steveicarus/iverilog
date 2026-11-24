// Check nonblocking assignments of interface members through a virtual
// interface that is held in an automatic variable.
interface bus_if;
  logic [7:0] data = 0;
  logic [7:0] mask = 0;
endinterface

class drv;
  virtual bus_if vif;
  task run(input logic [7:0] d);
    virtual bus_if l;
    l = vif;
    l.data <= d;
  endtask
endclass

module test;
  bus_if b0();
  bus_if b1();
  drv d;
  bit failed = 0;

  task automatic put(virtual bus_if v, int i);
    virtual bus_if l;
    l = v;
    l.mask[i] <= 1'b1;
    l.data <= #2 8'h5a;
  endtask

  initial begin
    d = new;
    d.vif = b0;
    d.run(8'h33);
    if (b0.data !== 8'h00) failed = 1;
    #1;
    if (b0.data !== 8'h33) failed = 1;
    put(b1, 3);
    if (b1.mask !== 8'h00) failed = 1;
    #1;
    if (b1.mask !== 8'h08) failed = 1;
    if (b1.data !== 8'h00) failed = 1;
    #2;
    if (b1.data !== 8'h5a) failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end
endmodule
