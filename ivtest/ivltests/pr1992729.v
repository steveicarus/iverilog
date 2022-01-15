module main;

   function integer my_ceil;
      input number;
      real number;
      if (number > $rtoi(number))
        my_ceil = $rtoi(number) + 1;
      else
        my_ceil = number;
   endfunction

   real      tck;
   parameter CL_TIME =   13125;
   wire [31:0] result1 = my_ceil( CL_TIME/tck );
   integer     result2;
   initial begin
      tck = 2.0;

      result2 = my_ceil( CL_TIME/tck );
      if (result2 !== 6563) begin
	 $display("FAILED -- result2=%d", result2);
	 $finish;
      end

      #1 if (result1 !== 6563) begin
	 $display("FAILED -- result1=%d", result1);
	 $finish;
      end
      $display("PASSED");
   end // initial begin

endmodule // main
