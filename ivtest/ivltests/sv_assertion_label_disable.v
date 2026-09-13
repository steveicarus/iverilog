// Check that disable can target an assertion label.

module test;

  reg [7:0] value;

  initial begin
    value = 0;
    CHECK: assert (1) begin
      value = 1;
      disable CHECK;
      value = 2;
    end

    if (value === 1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
