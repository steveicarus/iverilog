// error: a read through a handle of an interface with no instances
interface bus_if;
  logic data;
endinterface
module test;
  virtual bus_if v;
  logic x;
  initial x = v.data;  // compiled with -s test, so bus_if is not a root
endmodule
