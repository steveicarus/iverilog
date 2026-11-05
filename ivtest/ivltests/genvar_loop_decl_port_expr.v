// Check that a genvar declared in a generate loop header can be used in
// instance port expressions. The genvar must not be mistaken for an undeclared
// identifier, which would implicitly declare a net that hides the genvar.

module child(input wire [31:0] k, output wire [31:0] o);
  assign o = k;
endmodule

module stage(input wire [31:0] i, input wire [31:0] k, output wire [31:0] o);
  assign o = i + k;
endmodule

module test;
  reg [31:0] start;
  wire [2:0][31:0] c;
  reg failed;

  // Module port expression.
  for (genvar i = 0; i < 2; i = i + 1) begin : m
    wire [31:0] o;
    child u(.k(start ^ i), .o(o));
  end

  // Gate terminal expression.
  for (genvar i = 0; i < 2; i = i + 1) begin : g
    wire o;
    buf b(o, i == 1);
  end

  // Module port expression in a nested generate block.
  for (genvar i = 0; i < 2; i = i + 1) begin : n
    if (1) begin : blk
      wire [31:0] o;
      child u(.k(start ^ i), .o(o));
    end
  end

  // Constant select in an output port of an instance that also uses the
  // genvar in another port expression.
  assign c[0] = start;
  for (genvar i = 0; i < 2; i = i + 1) begin : s
    stage u(.i(c[i]), .k(start ^ i), .o(c[i+1]));
  end

  initial begin
    failed = 0;
    start = 8;
    #1;
    if (m[0].o !== 8 || m[1].o !== 9) begin
      $display("FAILED: module port, got %0d %0d", m[0].o, m[1].o);
      failed = 1;
    end
    if (g[0].o !== 1'b0 || g[1].o !== 1'b1) begin
      $display("FAILED: gate terminal, got %b %b", g[0].o, g[1].o);
      failed = 1;
    end
    if (n[0].blk.o !== 8 || n[1].blk.o !== 9) begin
      $display("FAILED: nested block, got %0d %0d", n[0].blk.o, n[1].blk.o);
      failed = 1;
    end
    if (c[1] !== 16 || c[2] !== 25) begin
      $display("FAILED: output select, got %0d %0d", c[1], c[2]);
      failed = 1;
    end
    if (!failed)
      $display("PASSED");
  end
endmodule
