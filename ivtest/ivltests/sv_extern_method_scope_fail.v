// The definition must be in the scope that declares the class.

module test;
  class C;
    extern function int f();
  endclass

  if (1) begin : gen
    function int C::f();
      return 1;
    endfunction
  end
endmodule
