// sorry: a virtual interface type with a modport
interface bus_if;
  logic data;
  modport mst (output data);
endinterface
module test;
  bus_if b1();
  virtual bus_if.mst v;
endmodule
