// $abs should take a real argument and return a real result.
module test();

localparam s = 0;
localparam a = 1.5;
localparam b = 1;
localparam r = $abs((s ? a : b) / 2);

initial begin
  $display("%g", r);
  if (r == 0.5)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
