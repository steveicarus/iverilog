`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass;
  wire [2:-1] vec;

  assign vec = 4'bxxxx;

  initial begin
    pass = 1'b1;

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    force vec[1'bx] = 1'b1;
`endif
    if (vec !== 4'bxxx) begin
      $display("Failed vec[1'bx], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    release vec[1'bx];
`endif

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    force vec[1'bx:0] = 1'b1;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx:0], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    release vec[1'bx:0];
`endif

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    force vec[0:1'bx] = 1'b1;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[0:1'bx], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    release vec[0:1'bx];
`endif

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    force vec[1'bx:1'bx] = 1'b1;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx:1'bx], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    release vec[1'bx:1'bx];
`endif

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    force vec[1'bx+:1] = 1'b1;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx+:1], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    release vec[1'bx+:1];
`endif

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    force vec[1'bx+:2] = 2'b01;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx+:2], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    release vec[1'bx+:2];
`endif

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    force vec[1'bx-:1] = 1'b1;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx-:1], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    release vec[1'bx-:1];
`endif

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    force vec[1'bx-:2] = 2'b01;
`endif
    if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx-:2], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    release vec[1'bx-:2];
`endif

    if (pass) $display("PASSED");
  end
endmodule
