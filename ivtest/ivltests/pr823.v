module main;

   reg [31:0] foo;
   initial foo = 'b0;

   initial begin
      #1 if (foo !== 32'd0) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
