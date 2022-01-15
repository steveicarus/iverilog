module top;
  wire real result;
  wire [63:0] bits;
  real in;

  assign bits = $realtobits(in);

  // This generates incorrect code:
  // The .net/real temporary is not needed.
  // The .alias/real temporary is not needed.
  // The .sfunc should connect directly to the "results" net.
  // The .part is not needed and is causing a core dump.
  //
  // Once these are fixed it appears there is a concurrency issues
  assign result = $bitstoreal(bits);

  initial begin
    $monitor(result,, bits,, in);

       in = 0.0;
    #1 in = 2.0;
  end
endmodule
