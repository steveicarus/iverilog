// The ternary operator should cause the 'in' variable to be unsigned extended.
module test;
  reg passed;
  reg [7:0] res;
  reg signed in;

  initial begin
    passed = 1'b1;
    in = 1;
    $display("in4: %0d", in);

    res = 0 ? 1'h0 : in;
    $display("T0 = %d, %d", 0 ? 1'h0 : in, res); // These work
    case (0 ? 1'h0 : in) // But this fails
      5'b0101: begin $display("FAILED: T0 matched 5'b0101"); passed = 1'b0;end
      8'b000001: begin $display("T0 matched 8'b000001"); end
      default: begin $display("FAILED: T0 matched default"); passed = 1'b0;end
    endcase

    res = 1 ? in : 1'h0;
    $display("T1 = %d, %d", 1 ? in : 1'h0, res); // These work
    case (1 ? in : 1'h0) // But this fails
      5'b0101: begin $display("FAILED: T1 matched 5'b0101"); passed = 1'b0;end
      8'b000001: begin $display("T1 matched 8'b000001"); end
      default: begin $display("FAILED: T1 matched default"); passed = 1'b0;end
    endcase

    if (passed) $display("PASSED");
  end
endmodule
