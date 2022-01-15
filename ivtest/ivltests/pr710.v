module main;

   reg [5:0] idx, mask;
   wire [5:0] foo = idx & mask;

   initial begin
      mask = 5'h1f;
      for (idx = 0 ;  idx < 5 ;  idx = idx+1)
	wait (foo == idx) begin
	   $display("foo=%d, idx=%d", foo, idx);
	   if (foo !== idx) begin
	      $display("FAILED");
	      $finish;
	   end
	end

      $display("PASSED");
   end

endmodule // main
