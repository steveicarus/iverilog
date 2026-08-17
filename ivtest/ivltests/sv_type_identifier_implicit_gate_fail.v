// Check that a type identifier does not become an implicit primitive terminal.

package p;
  typedef logic value;
endpackage

module test;
  import p::value;
  wire result;
  buf (result, value);
endmodule
