// sorry: a member through a handle in a concatenation l-value
interface bus_if;
  logic [3:0] data;
endinterface
module test;
  bus_if b1();
  virtual bus_if v;
  logic [3:0] x;
  initial {v.data, x} = 8'h12;
endmodule
