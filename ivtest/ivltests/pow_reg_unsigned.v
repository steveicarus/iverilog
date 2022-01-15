`begin_keywords "1364-2005"
module test();

reg  [15:0] a;
reg   [7:0] b;
reg  [31:0] expect;
reg  [31:0] actual;

reg [127:0] long_x;
real        real_x;

initial begin
  for (a = 0; a < 65535; a = a + 1) begin:outer_loop
    long_x = 1;
    for (b = 0; b < 127; b = b + 1) begin:inner_loop
      real_x = $itor(a) ** $itor(b);
      if (real_x >= 2.0**128.0) disable outer_loop;
      expect = long_x;
      actual = a ** b;
      actual = a ** b;
      actual = a ** b;
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
