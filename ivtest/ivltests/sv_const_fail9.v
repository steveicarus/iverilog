// Check that blocking assignment to a const variable fails, when the variable is
// declared with the `var` keyword.

module test;

  const var integer x = 10;

  initial begin
    x = 20; // Error: Assignment to const variable
    $display("FAILED");
  end

endmodule
