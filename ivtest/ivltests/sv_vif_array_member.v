// Check that members of an interface that has unpacked array members can be
// accessed through a virtual interface.
interface bus_if;
  int a_mem [0:3];
  logic [7:0] data;
  real r;
  string s;
  int z_mem [0:1];
endinterface

module test;
  bus_if b0();
  virtual bus_if v;
  bit failed = 0;

  initial begin
    v = b0;
    v.data = 8'h5a;
    v.r = 2.5;
    v.s = "hi";
    if (b0.data !== 8'h5a || v.data !== 8'h5a) failed = 1;
    if (b0.r != 2.5 || v.r != 2.5) failed = 1;
    if (b0.s != "hi" || v.s != "hi") failed = 1;
    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end
endmodule
