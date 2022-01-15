module top;
  reg passed = 1'b1;

  reg [199:0] a, b;
  wire [199:0] r;

  assign #1 r = a ** b;

  initial begin
    a = 'd5;

    b = 'd2; // A simple test.
    #2;
    if (r != 'd25) begin
      $display("Failed: 5 ** 2 gave %d, expected 25", r);
      passed = 1'b0;
    end

    b = 'd55; // A 128 bit value.
    #2;
    if (r != 200'd277555756156289135105907917022705078125) begin
      $display("Failed: 5 ** 55\n  gave     %0d", r);
      $display("  expected 277555756156289135105907917022705078125");
      passed = 1'b0;
    end

    b = 'd86; // A 200 bit value.
    #2;
    if (r != 200'd1292469707114105741986576081359316958696581423282623291015625) begin
      $display("Failed: 5 ** 55\n  gave     %0d", r);
      $display("  expected 1292469707114105741986576081359316958696581423282623291015625");
      passed = 1'b0;
    end
    if (r != 'd5**'d86) begin
      $display("Failed: compile-time/run-time value mismatch.");
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
