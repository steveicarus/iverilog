// Check that a real constant can be used as a wait condition.

module test;

  reg positive;
  reg negative;
  reg zero;

  initial begin
    positive = 1'b0;
    wait (0.25);
    positive = 1'b1;
  end

  initial begin
    negative = 1'b0;
    wait (-0.25);
    negative = 1'b1;
  end

  initial begin
    zero = 1'b0;
    wait (0.0);
    zero = 1'b1;
  end

  initial begin
    #1;
    if ({positive, negative, zero} === 3'b110) begin
      $display("PASSED");
    end else begin
      $display("FAILED: expected 110, got %b%b%b", positive, negative, zero);
    end
  end

endmodule
