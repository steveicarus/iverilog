module top;
  reg[63:0] a;

  initial begin
    a = 64'h7fe8000000000000;
    // This used to fail because we printed floating point using
    // the default buffer which was only 256 bytes long. To fix
    // this the default size was changed to 512 bytes and this is
    // increased when needed (%400.300f, etc.).
    $display("%6.3f", $bitstoreal(a));
    $display("PASSED");
  end
endmodule
