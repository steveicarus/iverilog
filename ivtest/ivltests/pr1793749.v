module test;
   reg [8:0] t1;
   initial
     main;

   function integer log2;
      input [31:0] arg;
      for (log2=0; arg > 0; log2=log2+1)
	arg = arg >> 1;
   endfunction // log2

   task main;
      integer temp;
      begin
	 t1 = 9'h0a5;
	 temp = log2($unsigned(t1 - t1 - 1'b1));
	 $display("%d", temp);

	 temp = log2($signed(t1 - t1 - 1'b1));
	 $display("%d", temp);

	 temp = log2({t1 - t1 - 1'b1});
	 $display("%d", temp);

	 temp = $bits(t1 - t1 - 1'b1);
	 $display("%d", temp);
      end
   endtask
endmodule
