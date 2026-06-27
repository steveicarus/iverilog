// Check that a declaration after a null statement is rejected.

module test;

  initial begin
    ;
    integer value;
  end

endmodule
