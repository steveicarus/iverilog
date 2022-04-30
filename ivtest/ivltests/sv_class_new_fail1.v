// Check that using the class new operator on a non-class variable results in an
// error.

module test;

  int i;

  initial begin
    i = new;
    $display("FAILED");
  end

endmodule
