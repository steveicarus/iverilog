`timescale 1ns/1ns

module test();

   reg [4 : 0] A = 5'b0;
   reg	       CLK = 1'b0;

   integer     pipe;


   initial
     begin
	#2000 if (A !== 0)
	  $display("FAILED");
	else
	  $display("PASSED");
	$finish;
     end
   always #20 CLK = !CLK;

   always @(posedge CLK)
     for(pipe = 2; pipe <= -1; pipe = pipe + 1)
       A<=A+1;

endmodule // test
