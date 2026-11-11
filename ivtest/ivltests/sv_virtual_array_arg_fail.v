// Check that an unpacked array argument of an overridden virtual method
// is reported as not supported instead of failing in the code generator.

module test;

  class C;
    virtual function int sum(int a[3]);
      return a[0] + a[1] + a[2];
    endfunction
  endclass

  class D extends C;
    function int sum(int a[3]);
      return 0;
    endfunction
  endclass

endmodule
