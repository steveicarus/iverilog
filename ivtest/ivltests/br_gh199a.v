module bug();
  reg [31:0] n1, d1, q1, m1;
  reg [63:0] n2, d2, q2, m2;

  initial begin
    n1 = 32'h8000_0000;
    d1 = 32'hFFFF_FFFF;
    q1 = $signed(n1) / $signed(d1);
    $display("32 bit quotient = 0x%08h;", q1);
    m1 = $signed(n1) % $signed(d1);
    $display("32 bit modulus  = 0x%08h;", m1);
    n2 = 64'h8000_0000_0000_0000;
    d2 = 64'hFFFF_FFFF_FFFF_FFFF;
    q2 = $signed(n2) / $signed(d2);
    $display("64 bit quotient = 0x%016h;", q2);
    m2 = $signed(n2) % $signed(d2);
    $display("64 bit modulus  = 0x%016h;", m2);
    if ((q1 === 32'h8000_0000) && (q2 === 64'h8000_0000_0000_0000)
    &&  (m1 === 32'h0000_0000) && (m2 === 64'h0000_0000_0000_0000))
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule
