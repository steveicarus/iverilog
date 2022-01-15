module top;
  reg [63:0] str;
  reg [31:0] in, out;
  integer res;

  initial begin
    // To avoid embedded NULL bytes each byte must have an x or a 1 and a z.
    in = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $sformat(str, "%z", in);
    res = $sscanf(str, "%z", out);
    if (res !== 1) $display("FAILED: $sscanf() returned %d", res);
    else if (in !== out) $display("FAILED: %b !== %b", in, out);
    else $display("PASSED");
  end
endmodule
