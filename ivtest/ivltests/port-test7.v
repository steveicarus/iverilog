`define REG_DELAY 1

module ansireg(input clk, reset, input [7:0] d, output reg [7:0] q );
always @(posedge clk or posedge reset)
   if(reset)
     q <= #(`REG_DELAY) 8'h00;
   else
     q <= #(`REG_DELAY) d;

endmodule

module main;

   reg clk, reset;
   reg [7:0] d;
   wire [7:0] q;

   ansireg U(clk, reset, d, q);

   initial begin
      clk = 0;
      reset = 0;
      d = 'hff;

      #(2*`REG_DELAY) clk <= 1;
      #(2*`REG_DELAY) if (q !== d) begin
	 $display("FAILED -- clk=%b, reset=%b, d=%b, q=%b", clk, reset, d, q);
	 $finish;
      end

      reset <= 1;
      #(1 + `REG_DELAY) if (q !== 8'h00) begin
	 $display("FAILED -- clk=%b, reset=%b, d=%b, q=%b", clk, reset, d, q);
	 $finish;
      end

      $display("PASSED");
   end // initial begin
endmodule // main
