module ternary;

   wire [5:0] a;
   wire [6:0] b;
   wire       c;

   wire [5:0] d = c ? a : b;

   initial
     begin
	$display("PASSED");
	$finish;
     end

endmodule
