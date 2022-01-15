module main;

   reg [3:0]  value;
   reg [2:0]  addr;

   wire       test_bit = value[addr] == 1;

   initial begin
      value = 'b0110;

      for (addr = 0 ;  addr < 4 ;  addr = addr+1) begin
	 #1 if (test_bit !== value[addr]) begin
	    $display("FAILED -- value[%d]=%b, test_bit=%b",
		     addr, value[addr], test_bit);
	    $finish;
	 end
      end

      $display("PASSED");
   end // initial begin

endmodule // main
