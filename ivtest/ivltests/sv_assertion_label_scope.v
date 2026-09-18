// Check that assertion labels introduce separate scopes for action blocks.

module test;

  initial begin
    A: assert (1) begin : B
      reg [7:0] value;
      value = 1;
    end

    C: assume (0) else begin : B
      reg [7:0] value;
      value = 2;
    end

    if (test.A.B.value === 1 && test.C.B.value === 2) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
