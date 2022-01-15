`timescale 1ns/1ns

module test();

   reg [4 : 0] A = 5'b0;
   reg	       CLK = 1'b0;

   parameter   stages = 0;

   integer     pipe;


   initial
     begin
	#2000 $display("PASSED");
	$finish;
     end

   always #20 CLK = !CLK;

   always @(posedge CLK)
     for(pipe = 2; pipe <= stages -1; pipe = pipe + 1) begin
	$display("FAILED");
	$finish;
	A<=A+1;
     end

endmodule // test
