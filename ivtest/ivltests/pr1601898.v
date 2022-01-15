module test;
   initial begin
      main;
      $display("PASSED");
   end

   task main;
      begin
	 if(1)
	   ;
	 else begin
	    $display("FAILED");
	    $finish;
	 end
      end
   endtask
endmodule
