// Regression test for GitHub issue 13 : Icarus Verilog creates huge in-memory
// arrays for shifts with large rhs. This version checks behaviour for unsized
// expressions.

module bug();

localparam p = 1 << ~40'b0;

initial begin
  $display("%0d", p);
  if (p === 5'b00000)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
