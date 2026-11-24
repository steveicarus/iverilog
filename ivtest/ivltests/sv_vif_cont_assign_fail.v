// error: a virtual interface member in a continuous assignment (1800-2017 25.9)
interface bus_if;
  logic [7:0] data;
endinterface
module test;
  bus_if b1();
  virtual bus_if v;
  wire [7:0] w = v.data;
  wire [7:0] y;
  assign y = v.data + 1;
endmodule
