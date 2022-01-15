module top;
  wire real result;
  reg [63:0] bits;

  // This generates incorrect code:
  // The .net/real temporary is not needed.
  // The .alias/real temporary is not needed.
  // The .sfunc should connect directly to the "results" net.
  // The .part is not needed and is causing a core dump.
  assign result = $bitstoreal(bits);

  initial begin
    $monitor("%g %h", result, bits);

       bits = 64'b0;
    #1 bits = {1'b0,{62{1'b1}}};
  end
endmodule
