// Check compound assignments to part selects with negative offsets.

module test;

  reg [7:0] value;
  reg signed [2:0] base;

  initial begin
    value = 8'h30;
    base = -2;
    value[base +: 4] ^= 4'b1100;
    if (value !== 8'h33) begin
      $display("FAILED: expected 33, got %h", value);
    end else begin
      $display("PASSED");
    end
  end

endmodule
