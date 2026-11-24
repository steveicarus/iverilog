// an event wait through a null handle stops the simulation with an error
interface bus_if;
  logic clk;
endinterface

module test;
  bus_if b1();
  virtual bus_if v;

  initial begin
    @(posedge v.clk);
    $display("FAILED: wait through null handle continued");
  end
  initial #10 $display("FAILED: no error");
endmodule
