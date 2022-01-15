module t();

   reg passed = 1;

   task abc;
      input [7:0] a;

      begin
	 if(a == 1)
	   $display("OK");
	 else
	   begin $display("FAILURE"); passed = 0; end
      end
   endtask

   reg [7:0] b;

   initial
     begin
	#1 ;
	abc(500 >> 8);
	b = 500 >> 8;
	abc(b);

	if (passed)
	  $display("PASSED");
     end
endmodule
