// Check that pure virtual function declarations produce a sorry error.
// IEEE 1800-2012: A.1.8 — pure virtual methods are not yet supported.
// A pure virtual declaration is a prototype with no body.

module test;

  class C;
    pure virtual function int get_val();
  endclass : C

endmodule : test
