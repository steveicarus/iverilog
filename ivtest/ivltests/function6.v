module test;

   function f_0;
      input i;
      begin
	 f_0 = f_1(i);
      end
   endfunction

   function f_1;
      input i;
      begin
	 f_1 = !i;
      end
   endfunction

   wire w = f_0(1'b0);

    initial begin
	#1;
	if ( w !== 1'b1)
	    $display ("FAILED w (%b) !== 1'b1", w);
	else
	    $display ("PASSED");
    end
endmodule
