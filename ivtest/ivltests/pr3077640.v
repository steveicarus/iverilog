module top;
  reg pass;
  reg [7:0] val;
  reg signed [7:0] sval;

  initial begin
    pass = 1'b1;

    // An unsized number has an implicit width of integer width.
    val = $unsigned(-4);
    if (val !== 8'hfc) begin
      $display("Failed unsigned, expected 8'hfc, got %h", val);
      pass = 1'b0;
    end

    val = $unsigned(-4'sd4);
    if (val !== 8'h0c) begin
      $display("Failed sized unsigned, expected 8'h0c, got %h", val);
      pass = 1'b0;
    end

    sval = $signed(4'hc);
    if (sval !== -4) begin
      $display("Failed signed, expected -4, got %d", sval);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
