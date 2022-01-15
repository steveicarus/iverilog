`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass;
  reg [2:-1] vec;
  integer idx;

  initial begin
    pass = 1'b1;

    idx = 'bx;

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    vec[1'bx] <= 1'b1;
`endif
    #1 if (vec !== 4'bxxx) begin
      $display("Failed vec[1'bx], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
    vec[idx] <= 1'b1;
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    vec[1'bx:0] <= 1'b1;
`endif
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx:0], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    vec[0:1'bx] <= 1'b1;
`endif
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[0:1'bx], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    vec[1'bx:1'bx] <= 1'b1;
`endif
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx:1'bx], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    vec[1'bx+:1] <= 1'b1;
`endif
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx+:1], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    vec[1'bx+:2] <= 2'b01;
`endif
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx+:2], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    vec[1'bx-:1] <= 1'b1;
`endif
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx-:1], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    vec[1'bx-:2] <= 2'b01;
`endif
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[1'bx-:2], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
    vec[idx+:1] <= 1'b1;
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[idx+:1], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
    vec[idx+:2] <= 2'b01;
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[idx+:2], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
    vec[idx-:1] <= 1'b1;
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[idx-:1], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    vec = 4'bxxxx;
    vec[idx-:2] <= 2'b01;
    #1 if (vec !== 4'bxxxx) begin
      $display("Failed vec[idx-:2], expected 4'bxxxx, got %b", vec);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
