/*
 * See pr245 in the ivtest test suite.
 */

`timescale 1ns/1ns

module t;
   wire [11:0] iodata;
   integer     i;


   initial
     begin
        $timeformat(-9,0,"ns",5);
	$display("   TIME:IOD");
	$monitor(    "%7t:%3x",
		     $time,iodata);
	#0
	  force iodata =0;
	for (i=0; i<512;i=i+1)
	  #10
	       force iodata =i;
     end // initial begin
endmodule // t
