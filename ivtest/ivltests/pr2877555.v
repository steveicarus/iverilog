module testbench;
  // This should give us a vector of [-1:0] (A == 0).
  parameter A = $clog2(1);
  wire [A-1:0] x;
  wire [A-1:0] y = x;

  // Check to see that we got a two bit wide wire.
  initial if ($bits(x) == 2) $display("PASSED");
    else $display("FAILED");
endmodule
