// Check that void casting an expression results in an error

module test;

  initial begin
    void'(1+2);
  end

endmodule
