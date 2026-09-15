// Check that a real net propagates its initial zero to downstream logic.

module test;

  wire real value = 0.0;
  wire [63:0] bits = $realtobits(value);

  initial begin
    #1;
    if (bits === 64'b0) begin
      $display("PASSED");
    end else begin
      $display("FAILED: expected zero bits, got %h", bits);
    end
  end

endmodule
