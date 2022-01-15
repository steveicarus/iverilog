`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass;
  reg [7:0] in;
  reg [3:0] out;

  initial begin
    pass = 1'b1;
    in = 8'b10100101;

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    out = in[7:'dx];
`else
    out = 4'bxxxx;
`endif
    if (out !== 4'bxxxx) begin
      $display("FAILED: part select LSB is X, expected 4'bxxxx, got %b", out);
      pass = 1'b0;
    end

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    out = in['dx:0];
`else
    out = 4'bxxxx;
`endif
    if (out !== 4'bxxxx) begin
      $display("FAILED: part select MSB is X, expected 4'bxxxx, got %b", out);
      pass = 1'b0;
    end

    out = 4'b0000;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    out[0] = in['dx];
`else
    out[0] = 1'bx;
`endif
    if (out !== 4'b000x) begin
      $display("FAILED: bit select is X, expected 4'b000x, got %b", out);
      pass = 1'b0;
    end

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    out = in[7:'dz];
`else
    out = 4'bxxxx;
`endif
    if (out !== 4'bxxxx) begin
      $display("FAILED: part select LSB is Z, expected 4'bxxxx, got %b", out);
      pass = 1'b0;
    end

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    out = in['dz:0];
`else
    out = 4'bxxxx;
`endif
    if (out !== 4'bxxxx) begin
      $display("FAILED: part select MSB is Z, expected 4'bxxxx, got %b", out);
      pass = 1'b0;
    end

    out = 4'b0000;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    out[0] = in['dz];
`else
    out[0] = 1'bx;
`endif
    if (out !== 4'b000x) begin
      $display("FAILED: bit select is Z, expected 4'b000x, got %b", out);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
