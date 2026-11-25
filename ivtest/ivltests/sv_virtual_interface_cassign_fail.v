// Check that a virtual interface member in a continuous assignment is an error.

interface bus_if;
  logic [7:0] data = 0;
endinterface
module test;
  bus_if b0();
  virtual bus_if v;
  wire [7:0] w;
  assign w = v.data;
  initial v = b0;
endmodule
