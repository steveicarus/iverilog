// Check that a typedef shadows an outer value in a typed expression.

class C;
endclass

module test;

  C value;
  C result;

  initial begin : inner
    typedef logic value;
    result = value;
  end

endmodule
