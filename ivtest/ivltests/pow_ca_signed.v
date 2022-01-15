`begin_keywords "1364-2005"
module test();

reg  signed  [15:0] a;
reg  signed   [7:0] b;
reg  signed  [31:0] expect;
wire signed  [31:0] actual;

reg  signed [127:0] long_x;
real                real_x;

assign actual = a ** b;

initial begin
  for (a = -32768; a < 32767; a = a + 1) begin:outer_loop
    long_x = 1;
    for (b = 0; b < 127; b = b + 1) begin:inner_loop
      real_x = $itor(a) ** $itor(b);
      if (real_x < 0.0) real_x = -real_x;
      if (real_x >= 2.0**128.0) disable outer_loop;
      expect = long_x;
      #0; // wait for net propagation
      if (actual !== expect) begin
        $display("FAILED : %0d ** %0d = %0d not %0d", a, b, expect, actual);
        $finish;
      end
      long_x = long_x * a;
    end
  end
  $display("PASSED");
end

endmodule
`end_keywords
