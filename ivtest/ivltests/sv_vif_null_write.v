// a write through a null handle stops the simulation with an error
interface bus_if;
  logic [7:0] data;
endinterface

module test;
  bus_if b1();
  virtual bus_if v;

  initial begin
    v = b1;
    v.data = 1;
    v = null;
    v.data = 2;
    $display("FAILED: write through null handle continued");
  end
endmodule
