`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass;
  reg [2:-1] vec;

  initial begin
    pass = 1'b1;

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    assign vec[1'bx] = 1'b1;
`endif
    if (vec !== 4'bxxx) begin
      $display("Failed vec[1'bx], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    deassign vec[1'bx];
`endif

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    assign vec[1'bx:0] = 1'b1;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx:0], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    deassign vec[1'bx:0];
`endif

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    assign vec[0:1'bx] = 1'b1;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[0:1'bx], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    deassign vec[0:1'bx];
`endif

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    assign vec[1'bx:1'bx] = 1'b1;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx:1'bx], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    deassign vec[1'bx:1'bx];
`endif

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    assign vec[1'bx+:1] = 1'b1;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx+:1], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    deassign vec[1'bx+:1];
`endif

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    assign vec[1'bx+:2] = 2'b01;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx+:2], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    deassign vec[1'bx+:2];
`endif

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    assign vec[1'bx-:1] = 1'b1;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx-:1], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    deassign vec[1'bx-:1];
`endif

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    assign vec[1'bx-:2] = 2'b01;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx-:2], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    deassign vec[1'bx-:2];
`endif

    if (pass) $display("PASSED");
  end
endmodule
