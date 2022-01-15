/*
 * This is a reuced version of PR#990, that captures the essence.
 * Or at least the bug being reported.
 */
module bug();

reg [31:0] x;
wire y;

assign y = x == 0;

initial begin
  $display("y: %b", y);
  x = 0;
  #0;
  $display("y: %b", y);
  if (y === 1'b1) // if x is 0, then x==0 is 1.
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule // bug
