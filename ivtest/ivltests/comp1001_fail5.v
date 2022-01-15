module top;
  reg passed;
  reg [7:0] res;
  reg zero;
  reg one;
  initial begin
    passed = 1'b1;
    zero = 1'b0;
    one = 1'b1;
//    res = 1'b1 ? !1'b0/1'b0 : 1'b0;
    res = !1'b0/1'b0;
    if (res !== 8'bx) begin
      $display("FAILED: compiler.");
      passed = 1'b0;
    end

//    res = one ? !1'b0/zero : zero;
    res = !1'b0/zero;
    if (res !== 8'bx) begin
      $display("FAILED: run-time.");
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
