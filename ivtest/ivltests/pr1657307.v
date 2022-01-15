module test;
   integer dindex[2:0];
   integer cindex[2:0];

   initial
     main;

   task main;
      integer index;
      begin
	 dindex[0] = 3;
	 dindex[1] = 4;
	 dindex[2] = 5;

	 cindex[0] = 1;
	 cindex[1] = 2;
	 cindex[2] = 3;

	 index = get_index(3);
	 $write("index is %0d\n", index);
	 if (index !== 32)
	   $display("FAILED");
	 else
	   $display("PASSED");
      end
   endtask

   function integer get_index;
      input rank;
      integer rank;
      integer i, sum, multiplier;
      begin
	 multiplier = 1;
	 sum = 0;
	 for(i = rank-1; i >= 0; i = i-1) begin
	    sum = sum + (cindex[i] * multiplier);
	    multiplier = dindex[i] * multiplier;
	 end
	 get_index = sum-1;
      end
   endfunction

endmodule // test
