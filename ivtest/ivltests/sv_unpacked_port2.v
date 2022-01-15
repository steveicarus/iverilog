
module test
  (input clk,
   input wire [7:0] D,
   input wire [1:0] S,
   output reg [7:0] Q [0:3]
   /* */);

   always @(posedge clk)
     Q[S] <= D;

endmodule // test


module main;

   reg clk;
   reg [1:0] S;
   reg [7:0] D;
   wire [7:0] Q [0:3];

   test dut(.clk(clk), .D(D), .S(S), .Q(Q));

   initial begin
      clk = 0;

      S = 0;
      D = 0;
      #1 clk = 1;
      #1 clk = 0;

      S = 1;
      D = 1;
      #1 clk = 1;
      #1 clk = 0;

      S = 2;
      D = 2;
      #1 clk = 1;
      #1 clk = 0;

      S = 3;
      D = 3;
      #1 clk = 1;
      #1 clk = 0;

      for (int idx = 0 ; idx < 4 ; idx = idx+1) begin
	 if (Q[idx] != idx) begin
	    $display("FAILED -- Q[%0d] = %0d", idx, Q[idx]);
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule // main
