module bug();
  localparam signed [31:0] n1 = 32'h8000_0000;
  localparam signed [31:0] d1 = 32'hFFFF_FFFF;
  localparam signed [31:0] q1 = n1 / d1;
  localparam signed [31:0] m1 = n1 % d1;

  localparam signed [63:0] n2 = 64'h8000_0000_0000_0000;
  localparam signed [63:0] d2 = 64'hFFFF_FFFF_FFFF_FFFF;
  localparam signed [63:0] q2 = n2 / d2;
  localparam signed [63:0] m2 = n2 % d2;

  initial begin
    $display("32 bit quotient = 0x%08h;", q1);
    $display("32 bit modulus  = 0x%08h;", m1);
    $display("64 bit quotient = 0x%016h;", q2);
    $display("64 bit modulus  = 0x%016h;", m2);
    if ((q1 === 32'h8000_0000) && (q2 === 64'h8000_0000_0000_0000)
    &&  (m1 === 32'h0000_0000) && (m2 === 64'h0000_0000_0000_0000))
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule
