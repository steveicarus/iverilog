/*
 * Basics tests of continuous assignment.
 */
module testbench();
  reg [3:0] a, b;
  integer   c, d;
  wire [3:0] x, y, z;

  assign x = a + b + b;
  assign y = b - a;
  assign z = a + (a * b);

  initial begin
    a <= 4'h2;
    b <= 4'h3;
    #1;
    if (x !== 4'h8)
      begin
        $display("FAILED -- 2 + 3 + 3 !== 8");
        $finish;
      end
    if (y !== 4'h1)
      begin
        $display("FAILED -- 3 - 2 !== 1");
        $finish;
      end
    if (z !== 4'h8)
      begin
        $display("FAILED -- 2 + (2 * 3) !== 8");
        $finish;
      end
    $display("PASSED");
  end

endmodule // testbench
