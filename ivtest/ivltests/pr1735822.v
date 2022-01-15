module test();

   reg [0:(8*6)-1] identstr= "PASSED";
   reg [7:0] identdata= 8'b0;
   integer   i;

   initial
     begin
//	$dumpfile("indexed_part.vcd");
//	$dumpvars;
     end

   initial
     begin
	for (i=0; i<6; i=i+1)
	  begin
	     #10 identdata = identstr[i*8 +:8];
	         $write("%c", identdata);
	  end
	$write("\n");
	$finish;
     end

endmodule // test
