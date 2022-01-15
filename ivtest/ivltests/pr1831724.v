// pr1831724

module test;
   reg [15:0] tmp1, tmp2;
   initial
     begin
	tmp1 = 9'bxxx000000;
	tmp2 = {9'bxxx000000};
	$display("tmp1: '%b'; tmp2: '%b'", tmp1, tmp2);
     end
endmodule
