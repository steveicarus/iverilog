module test;
   reg [4:0]  i;
   reg [7:0]  j, k, l;

   initial
     main;
   task main;
      begin
	 i = 5'h14;
	 j = $signed(i);      // works
	 k = $signed(5'h14);  // doesn't work
	 l = 5'sh14;          // works

	 $display("i, j, k, l: '%b', '%b', '%b', '%b'", i, j, k, l);
      end
   endtask
endmodule
