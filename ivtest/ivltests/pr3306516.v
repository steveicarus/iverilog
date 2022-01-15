// Must be run with -gspecify
module top;
  reg passed;
  reg a, b;
  wire y;

  initial begin
    passed = 1'b1;
    a = 0;
    b = 1;
    #2 if (y !== 1'bx && y !== 1'bz) begin
      $display("Failed: Initial value, expected 1'bx, got %b", y);
      passed = 1'b0;
    end
    #2 if (y !== 1'b0) begin
      $display("Failed: Initial value propagation, expected 1'b0, got %b", y);
      passed = 1'b0;
    end
    a = 1;
    #2 if (y !== 1'b0) begin
      $display("Failed: to hold initial value, expected 1'b0, got %b", y);
      passed = 1'b0;
    end
    #2 if (y !== 1'b1) begin
      $display("Failed: Final value propagation, expected 1'b1, got %b", y);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end

  my_and dut(y, a, b);
endmodule

module my_and(output wire y, input wire a, b);
  specify
    specparam ta = 1;
    specparam tb = 2;
  endspecify
  // A specparam is just like a parameter in this context. In reality
  // they can be overridden at run-time so the following should really
  // be an expression instead of just a constant 3, but for now 3 would
  // be acceptable. Specparams and the specify block need a major rework.
  assign #(ta+tb) y = a & b;
endmodule
