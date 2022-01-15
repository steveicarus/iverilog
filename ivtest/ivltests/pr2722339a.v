module top;
  reg pass;
  reg  [7:0] a, b;
  wire [15:0] ruu, rsu, rus, rss;
  reg signed [15:0] res;
  integer i;

  assign ruu = a / b;
  assign rsu = $signed(a) / b;
  assign rus = a / $signed(b);
  assign rss = $signed(a) / $signed(b);

  initial begin
    pass = 1'b1;

    // Run 1000 random vectors
    for (i = 0; i < 1000; i = i + 1) begin
      // Random vectors
      a = $random;
      b = $random;
      #1;

      // Check unsigned / unsigned.
      if (ruu !== a/b) begin
        $display("FAILED: u/u (%b/%b) gave %b, expected %b", a, b, ruu, a/b);
        pass = 1'b0;
      end
      // Check signed / unsigned.
      if (rsu !== a/b) begin
        $display("FAILED: s/u (%b/%b) gave %b, expected %b", a, b, rsu, a/b);
        pass = 1'b0;
      end
      // Check unsigned / signed.
      if (rus !== a/b) begin
        $display("FAILED: u/s (%b/%b) gave %b, expected %b", a, b, rus, a/b);
        pass = 1'b0;
      end
      // Check signed / signed.
      res = $signed(a)/$signed(b);
      if (rss !== res) begin
        $display("FAILED: s/s (%b/%b) gave %b, expected %b", a, b, rss, res);
        pass = 1'b0;
      end
    end

    if (pass) $display("PASSED");
  end
endmodule
