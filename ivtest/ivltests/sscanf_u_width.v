/*
 * Regression test for a heap buffer overflow in the %u format code of
 * $sscanf(). When the match width spans more 32-bit words than the
 * destination variable holds, only the words that fit were stored, but
 * the trailing width mask still accessed val_ptr[words-1], one or more
 * words past the end of the allocation.
 */
module top;
  reg [63:0] str;
  reg [31:0] in, ck, out;
  integer res;

  initial begin
    // To avoid embedded NULL bytes each byte must have a 1.
    in = 32'b000x100z_001z000x_101xxxzz_100z111x;
    ck = 32'b00001000_00100000_10100000_10001110;
    // Two words of %u data (eight bytes), scanned with a 64-bit width
    // into a 32-bit variable so the width spans more words than fit.
    $sformat(str, "%u%u", in, in);
    res = $sscanf(str, "%64u", out);
    if (res !== 1) $display("FAILED: $sscanf() returned %d", res);
    else if (ck !== out) $display("FAILED: %b !== %b", ck, out);
    else $display("PASSED");
  end
endmodule
