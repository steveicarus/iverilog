module top;
  reg passed;
  reg a[];

  initial begin
    passed = 1'b1;

    a = new[3];

    a[0] = 1'b0;
    a[1] = 1'b1;

    if (a[0] !== 1'b0) begin
      $display("Failed a[0], expected 1'b0, got %b", a[0]);
      passed = 1'b0;
    end

    if (a[1] !== 1'b1) begin
      $display("Failed a[1], expected 1'b1, got %b", a[1]);
      passed = 1'b0;
    end

    if (a[2] !== 1'bx) begin
      $display("Failed a[2], expected 1'bx, got %b", a[2]);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
