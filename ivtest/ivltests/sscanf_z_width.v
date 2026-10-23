/*
 * Regression test for a heap buffer overflow in the %z format code of
 * $sscanf(). When the match width spans more 32-bit words than the
 * destination variable holds, only the words that fit were stored, but
 * the trailing width mask still accessed val_ptr[words-1], one or more
 * words past the end of the allocation.
 */
module top;
  reg [127:0] str;
  reg [31:0] in, out;
  integer res;

  initial begin
    // To avoid embedded NULL bytes each byte must have an x or a 1 and a z.
    in = 32'b000x100z_001z000x_101xxxzz_100z111x;
    // Two words of %z data (sixteen bytes), scanned with a 64-bit width
    // into a 32-bit variable so the width spans more words than fit.
    $sformat(str, "%z%z", in, in);
    res = $sscanf(str, "%64z", out);
    if (res !== 1) $display("FAILED: $sscanf() returned %d", res);
    else if (in !== out) $display("FAILED: %b !== %b", in, out);
    else $display("PASSED");
  end
endmodule
