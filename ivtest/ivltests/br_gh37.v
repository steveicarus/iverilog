// Regression test for GitHub issue #37
module test;
  wire [5:0]  a;
  wire [15:0] y;

  assign a = ~0;
  assign y = 1 ? ~a >>> 5 : 0;

  initial begin
    #10 $display("%b", y);
    if (y === 16'b1111111111111110)
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule
