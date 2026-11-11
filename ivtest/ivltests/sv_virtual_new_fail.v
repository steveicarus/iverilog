// Check that a constructor cannot be declared virtual.
// IEEE 1800-2017 8.20: only ordinary methods can be virtual.

module test;

  class C;
    virtual function new();
    endfunction
  endclass

endmodule
