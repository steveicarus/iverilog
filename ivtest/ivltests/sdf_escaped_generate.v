`timescale 1ps/1ps
// Preserve generated-scope and indexed-port resolution while decoding escapes.
module driver(input A, output [1:0] Z);
  assign Z = {A, A};
endmodule
module receiver(input [1:0] A, output Z);
  buf (Z, A[1]);
endmodule
module top;
  reg A = 0;
  wire Z;
  integer count = 0;
  for (genvar i = 0; i < 1; i = i+1) begin: g
    wire [1:0] drive;
    driver d(A, drive);
    receiver r(drive, Z);
  end
  initial begin
    $sdf_annotate("ivltests/sdf_escaped_generate.sdf", top);
    #100 A = 1;
    #100 A = 0;
    #50;
    if (count != 2) $fatal(1, "FAIL generated-scope count %0d", count);
    $display("PASSED");
    $finish(0);
  end
  always @(Z) if ($time > 50) begin
    count = count + 1;
    if (!((Z === 1'b1 && $time == 103) ||
          (Z === 1'b0 && $time == 205)))
      $fatal(1, "FAIL generated-scope delivery at %0t", $time);
  end
endmodule
