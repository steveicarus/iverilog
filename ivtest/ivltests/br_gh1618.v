// Check that a variable index after a constant non-zero index into a
// multi-dimensional unpacked array is not truncated when the canonical
// address is calculated.

module test;
  logic [1:0] m [0:1][0:1];
  int i;
  integer k;
  wire [1:0] w = m[1][i];
  reg failed = 1'b0;

  initial begin
    m[0][0] = 2'd0;
    m[0][1] = 2'd1;
    m[1][0] = 2'd2;
    m[1][1] = 2'd3;

    i = 1;
    #1 if (w !== 2'd3) begin
      $display("FAILED: w=%b with i=1", w);
      failed = 1'b1;
    end

    i = 16;
    #1 if (w !== 2'bxx) begin
      $display("FAILED: w=%b with i=16", w);
      failed = 1'b1;
    end

    k = 16;
    if (m[1][k] !== 2'bxx) begin
      $display("FAILED: m[1][k]=%b with k=16", m[1][k]);
      failed = 1'b1;
    end

    if (!failed) $display("PASSED");
  end
endmodule
