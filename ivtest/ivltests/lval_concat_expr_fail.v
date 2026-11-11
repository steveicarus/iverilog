// Check that every operand of a procedural l-value concatenation is an l-value.

module test;
  reg x, y;

  initial begin
    {x, y + 1} = 0; // Error: An arithmetic expression is not a procedural l-value.
    $display("FAILED");
  end
endmodule
