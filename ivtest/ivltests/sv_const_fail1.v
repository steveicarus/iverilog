// Check that continuous assignment to a const variable fails.

module test;

  const integer x = 10;

  assign x = 20; // Error: Assignment to const variable

  initial begin
    $display("FAILED");
  end

endmodule
