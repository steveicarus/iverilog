// Regression test for GitHub issue 15 : Icarus does undef propagation of
// const adds incorrectly

module bug();

wire [3:0] y;

assign y = 4'bxx00 + 2'b00;

initial begin
  #0 $display("%b", y);
  if (y === 4'bxxxx)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
