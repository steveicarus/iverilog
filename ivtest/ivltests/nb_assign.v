`begin_keywords "1364-2005"
module top;
  reg pass = 1'b1;
  reg [1:0] var = 2'b0;
  real rvar = 0.0;
  integer delay = 3;

  initial begin
    // These should both happen at time 2.
    var <= #2 2'b01;
    rvar <= #2 1.0;
    #3 if (var !== 2'b01) begin
      $display("FAILED: constant delay (bits)");
      pass = 1'b0;
    end
    if (rvar != 1.0) begin
      $display("FAILED: constant delay (real)");
      pass = 1'b0;
    end

    // These should both happen at time 6.
    var <= #(delay) 2'b10;
    rvar <= #(delay) 2.0;
    #4 if (var !== 2'b10) begin
      $display("FAILED: calculated delay (bits)");
      pass = 1'b0;
    end
    if (rvar != 2.0) begin
      $display("FAILED: calculated delay (real)");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
`end_keywords
