// Regression test for GitHub issue 13 : Icarus Verilog creates huge in-memory
// arrays for shifts with large rhs.

module bug();

localparam [4:0] p = 1'b1 << ~40'b0;

initial begin
  $display("%b", p);
  if (p === 5'b00000)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
