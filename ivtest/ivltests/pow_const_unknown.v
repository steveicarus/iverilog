// Check that constant powers propagate X through multiplication.

module test;
  localparam [7:0] VALUE = (8'd0 ** -8'sd1) * 8'd0;

  initial begin
    if (VALUE === 8'hxx) begin
      $display("PASSED");
    end else begin
      $display("FAILED: expected xx, got %h", VALUE);
    end
  end
endmodule
