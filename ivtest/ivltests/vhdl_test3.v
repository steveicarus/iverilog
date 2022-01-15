module main;

   reg clk;
   reg [4:0] in;
   wire [15:0] out;

   dummy dut (.clk(clk), .\input (in[3:0]), .\output (out));

   initial begin
      clk = 0;
      for (in = 0 ; in <= 16 ; in = in+1) begin
	 #1 clk = 1;
	 #1 clk = 0;
	 $display("input = %b, output=%b", in[3:0], out);
      end
   end

endmodule // main
