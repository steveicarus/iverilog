module main;

   wire [15:0] out;
   reg [16:0]  in;

   mask dut (.\output (out), .\input (in[15:0]));

   wire [15:0] out_ref = in[15:0] & 16'haaaa;
   initial begin
      for (in = 0 ; in[16] == 0 ; in = in+1)
	#1 if (out !== out_ref) begin
	   $display("FAILED: in=%b, out=%b, out_ref=%b", in, out, out_ref);
	   $finish;
	end

      $display("PASSED");
   end

endmodule // main
