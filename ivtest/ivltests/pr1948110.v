module top;
  reg pass = 1'b1;
  reg a, b;
  real c, d;

  initial begin
    c = 0.0;
    d = 1.0;
    a = 1'b0;
    b = 1'b0;
    assign c = 6/(2 - d*(b & ~a) + d*(a & ~b));

    #1;
    if (c != 3.0) begin
      $display("FAILED, expected 3.0, got %f", c);
      pass = 1'b0;
    end

    a = 1'b1;
    b = 1'b0;
    assign c = 6/(2 - d*(b & ~a) + d*(a & ~b));
    #1;
    if (c != 2.0) begin
      $display("FAILED, expected 2.0, got %f", c);
      pass = 1'b0;
    end

    a = 1'b0;
    b = 1'b1;
    assign c = 6/(2 - d*(b & ~a) + d*(a & ~b));
    #1;
    if (c != 6.0) begin
      $display("FAILED, expected 6.0, got %f", c);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
