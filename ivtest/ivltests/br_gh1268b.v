module top;
  reg passed;
  reg in, out;

  not(out, in); // This is valid for SystemVerilog

  initial begin
    passed = 1'b1;

    in = 1'b0;
    #1 if (out != ~in) begin
      $display("Expected %b, got %b", ~in, out);
      passed = 1'b0;
    end

    in = 1'b1;
    #1 if (out != ~in) begin
      $display("Expected %b, got %b", ~in, out);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
