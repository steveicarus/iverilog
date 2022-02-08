// Check that implicit type in a parameter port list without `parameter`
// generates an error.

module test #(signed A = 1);
  initial begin
    $display("FAILED");
  end
endmodule
