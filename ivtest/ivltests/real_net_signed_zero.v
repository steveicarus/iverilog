// Check that real nets propagate a change from positive zero to negative zero.

module test;

  real source;
  wire real value = source / 1.0;
  wire [63:0] observed = $realtobits(value);

  initial begin
    source = 0.0;
    #1;
    source = $bitstoreal(64'h8000000000000000);
    #1;
    if (observed === 64'h8000000000000000) begin
      $display("PASSED");
    end else begin
      $display("FAILED: expected negative zero bits, got %h", observed);
    end
  end

endmodule
