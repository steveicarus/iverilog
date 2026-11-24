// sorry: a dynamic array member through a handle
interface bus_if;
  int q[];
endinterface
module test;
  bus_if b1();
  bus_if b2();
  virtual bus_if v;
  int x[];
  initial x = v.q;
endmodule
