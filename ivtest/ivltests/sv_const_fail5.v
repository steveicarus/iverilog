// Check that procedural continuous assignment to a const variable fails.

module test;

  const integer x = 10;

  initial begin
    assign x = 20; // Error: Assignment to const variable
    $display("FAILED");
  end

endmodule
