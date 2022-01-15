module top;
  reg [63:0] str;
  reg [31:0] in, ck, out;
  integer res;

  initial begin
    // To avoid embedded NULL bytes each byte must have a 1.
    in = 32'b000x100z_001z000x_101xxxzz_100z111x;
    ck = 32'b00001000_00100000_10100000_10001110;
    $sformat(str, "%u", in);
    res = $sscanf(str, "%u", out);
    if (res !== 1) $display("FAILED: $sscanf() returned %d", res);
    else if (ck !== out) $display("FAILED: %b !== %b", in, out);
    else $display("PASSED");
  end
endmodule
