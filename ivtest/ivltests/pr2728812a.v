`timescale 1ns/1ns

module sum_test;
   reg clk;
   wire [10:0] s;

   initial begin
      clk = 0;
      forever #10 clk = ~clk;
   end

   sum #(5, 8) sum (clk, {8'd10,8'd20,8'd30,8'd40,8'd50}, s);

   initial begin
      $display("Starting...");
      repeat (50) @(posedge clk);
      $display("sum = %d",s);
      if (s !== 150)
         $display("FAILED: expected 150, received %0d",s);
      else
         $display("PASSED");
      $finish;
   end
endmodule

module sum
  #(
    parameter n = 4,
    parameter width = 8,
    parameter log_n = $clog2(n)
    )
   (
    input clk,
    input [n*width-1:0]addends,
    output reg [log_n+width-1:0] s
    );

   generate
   if (n==1)
     always @(*) s = addends;
   else begin
      wire [$clog2(n/2)+width-1:0] a1;
      wire [$clog2(n-n/2)+width-1:0] a2;
      sum #(n/2, width) s0 (clk, addends[(n/2)*width-1:0], a1);
      sum #(n-n/2, width) s1 (clk, addends[n*width-1:(n/2)*width], a2);
      always @(posedge clk) s <= a1 + a2;
   end
   endgenerate

endmodule // sum
