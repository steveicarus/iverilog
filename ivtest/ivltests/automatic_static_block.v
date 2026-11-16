module automatic_static_block;
  reg clk = 0;
  integer n = 0;

  always #5 clk = ~clk;

  always @(posedge clk) begin
    automatic int half = n / 2;
    automatic int zero;
    automatic logic [3:0] unknown;
    automatic integer values [0:1];

    if (half !== n / 2 || zero !== 0 || unknown !== 4'bxxxx ||
        values[0] !== 32'bx || values[1] !== 32'bx)
      $fatal(1, "entry %0d: half=%0d zero=%0d unknown=%b values=%0d,%0d",
             n, half, zero, unknown, values[0], values[1]);
    zero = 1;
    unknown = 0;
    values[0] = 1;
    values[1] = 2;
    n <= n + 1;
    if (n == 4) begin
      if (half !== 2) $fatal(1, "half=%0d", half);
      $display("PASSED");
      $finish;
    end
  end
endmodule
