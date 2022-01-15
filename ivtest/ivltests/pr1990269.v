`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass = 1'b1;
  reg [7:0] val;

  initial begin
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    val[3:-4] = 8'h6f;
`else
    val[3:0] = 4'h6;
`endif
    if (val !== 8'hx6) begin
      $display("FAILED underflow, got %h, expected 8'hx6", val);
      pass = 1'b0;
    end

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    val[11:4] = 8'hfe;
`else
    val[7:4] = 4'he;
`endif
    if (val !== 8'he6) begin
      $display("FAILED overflow, got %h, expected 8'he6", val);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
