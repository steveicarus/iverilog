module testbench();
  wire [3:0] q;
  reg        clr, clk, enable;

  counter uut(q, clr, clk);

  always @(clk)
    if (enable)
      #1 clk <= !clk;

  initial begin
    enable <= 1;
    clk <= 0;
    clr <= 1;
    #2;
    clr <= 0;
    #7;
    enable <= 0;
    if (q == 4'b0011)
      $display("PASSED");
    else
      $display("FAILED -- counter not correct (%d)", q);
  end

endmodule // testbench

module counter(q, clr, clk);
  output [3:0] q;
  input clr, clk;
  reg [3:0] q;

  always @(posedge clk or posedge clr)
    if (clr)
      q <= 4'b0000;
    else
      q <= q + 1'b1;

endmodule // counter
