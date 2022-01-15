`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass;
  wire [2:-1] vec1;
  wire [2:-1] vec2;
  wire [2:-1] vec3;
  wire [2:-1] vec4;
  wire [2:-1] vec5;
  wire [2:-1] vec6;

  assign vec1 = 4'bxxxx;
  assign vec2 = 4'bxxxx;
  assign vec3 = 4'bxxxx;
  assign vec4 = 4'bxxxx;
  assign vec5 = 4'bxxxx;
  assign vec6 = 4'bxxxx;

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  assign vec1[1'bx] = 1'b1;

  assign vec2[1'bx:0] = 1'b1;
  assign vec3[0:1'bx] = 1'b1;

  assign vec4[1'bx:1'bx] = 1'b1;

  assign vec5[1'bx+:1] = 1'b1;
  assign vec6[1'bx-:1] = 1'b1;
`endif

  initial begin
    pass = 1'b1;

    if (vec1 !== 4'bxxx) begin
      $display("Failed vec1, expected 4'bxxxx, got %b", vec1);
      pass = 1'b0;
    end

    if (vec2 !== 4'bxxx) begin
      $display("Failed vec2, expected 4'bxxxx, got %b", vec2);
      pass = 1'b0;
    end

    if (vec3 !== 4'bxxx) begin
      $display("Failed vec3, expected 4'bxxxx, got %b", vec3);
      pass = 1'b0;
    end

    if (vec4 !== 4'bxxx) begin
      $display("Failed vec4, expected 4'bxxxx, got %b", vec4);
      pass = 1'b0;
    end

    if (vec5 !== 4'bxxx) begin
      $display("Failed vec5, expected 4'bxxxx, got %b", vec5);
      pass = 1'b0;
    end

    if (vec6 !== 4'bxxx) begin
      $display("Failed vec6, expected 4'bxxxx, got %b", vec6);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
