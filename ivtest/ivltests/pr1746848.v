module forBug();
   integer i;
   integer j;

   initial
     begin
	// This loop sets i=4 .. -1 which is an error
	for (i=4; i>-1; i=i-1)
	  $display("i=%d",i);

	// This loop sets j=4 .. 0 which is correct.
	for (j=4; j>=0; j=j-1)
	  $display("j=%d",j);
     end
endmodule // forBug
