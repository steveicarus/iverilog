// Check that reading a member through a null virtual interface gives x, and
// that writing a member through a null virtual interface is a run time error.

interface bus_if;
  logic [7:0] data = 0;
endinterface

module test;
  bus_if b0();
  virtual bus_if v;

  initial begin
    if (v != null || !(v == null)) $display("FAILED: not null");
    if (v.data !== 8'hxx) $display("FAILED: null read");
    v = b0;
    v.data = 8'h5a;
    v = null;
    if (v.data !== 8'hxx || b0.data !== 8'h5a) $display("FAILED: null read");
    $display("before");
    v.data = 8'h01;
    $display("FAILED: after");
  end
endmodule
