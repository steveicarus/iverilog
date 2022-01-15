/*
 * A D-type flip-flop to check synchronous logic works
 * correctly.
 */

module testbench;
  reg d, clk, rst, enable;
  wire q, q_bar;

  dff uut(q, q_bar, d, clk, rst);

  initial clk <= 0;

  always @(clk)
    if (enable)
      #1 clk <= !clk;

  initial begin
    enable <= 1;
    rst <= 1;
    d <= 1'bx;
    #2;
    if (q !== 0)
      begin
        $display("FAILED -- Not reset");
        $finish;
      end
    rst <= 0;
    d <= 1'b1;
    #2;
    if (q !== 1)
      begin
        $display("FAILED -- q not 1 as expected");
        $finish;
      end
    d <= 1'b0;
    #2;
    if (q !== 0)
      begin
        $display("FAILED -- q not 0 as expected");
        $finish;
      end
    rst <= 1;
    #2;
    enable <= 0;  // Alternative to using $finish
    $display("PASSED");
  end

endmodule // testbench

module dff(q, q_bar, d, clk, rst);
  output q, q_bar;
  input d, clk, rst;
  reg q;

  always @(posedge clk or posedge rst)
    if (rst)
      q <= 1'b0;
    else
      q <= d;

  not(q_bar, q);

endmodule // dff
