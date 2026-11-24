// sorry: a virtual interface type with parameters
interface bus_if #(parameter W = 8);
  logic [W-1:0] data;
endinterface
module test;
  bus_if #(8) b1();
  virtual bus_if #(8) v;
endmodule
