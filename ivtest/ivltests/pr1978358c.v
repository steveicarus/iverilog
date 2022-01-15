`begin_keywords "1364-2005"

`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass = 1'b1;
//  reg [3:0] var = 4'b1001;
//  wire [3:0] var = 4'b1001;
  parameter [3:0] var = 4'b1001;
  reg [3:0] part;
  reg [5:0] big;

  initial begin
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = var[1:-2];  // should be 01xx.
    if (part !== 4'b01xx) begin
      $display("part select [1:-2] failed, expected 4'b01xx, got %b", part);
      pass = 1'b0;
    end

    part = var[5:2];  // should be xx10.
    if (part !== 4'bxx10) begin
      $display("part select [5:2] failed, expected 4'bxx10, got %b", part);
      pass = 1'b0;
    end

    big = var[4:-1];  // should be x10100101x.
    if (big !== 6'bx1001x) begin
      $display("part select [4:-1] failed, expected 6'bx1001x, got %b", big);
      pass = 1'b0;
    end
`endif

    if (pass) $display("PASSED");
  end
endmodule
`end_keywords
