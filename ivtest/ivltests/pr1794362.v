module shift;
  reg pass = 1'b1;
  integer i;
  reg [7:0] foo;
  parameter [2:0]
    ZERO = 0,
    THREE = 3;

  initial begin
    foo = 1'b1 << ZERO;
    if (foo != 'd1) begin
      $display("FAILED shift by ZERO, got %d expected 1", foo);
      pass = 1'b0;
    end

    foo = 1'b1 << THREE;
    if (foo != 'd8) begin
      $display("FAILED shift by THREE, got %d expected 8", foo);
      pass = 1'b0;
    end

    foo = 1'b1 << 3;
    if (foo != 'd8) begin
      $display("FAILED shift by 3, got %d expected 8", foo);
      pass = 1'b0;
    end

    foo = 1'b1 << 'd3;
    if (foo != 'd8) begin
      $display("FAILED shift by 'd3, got %d expected 8", foo);
      pass = 1'b0;
    end

    i = 'd3;
    foo = 1'b1 << i;
    if (foo != 'd8) begin
      $display("FAILED shift by variable set to 'd3, got %d expected 8", foo);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
