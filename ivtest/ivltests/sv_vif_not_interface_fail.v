// error: the name of a virtual interface type is not an interface
module nosuch;
endmodule
module test;
  virtual nosuch v;
endmodule
