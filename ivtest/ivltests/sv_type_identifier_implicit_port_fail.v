// Check that a type identifier does not become an implicit module connection.

package p;
  typedef logic value;
endpackage

module M(input wire data);
endmodule

module test;
  import p::*;
  M i_m(value);
endmodule
