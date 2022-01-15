module main;

   // The declaration assignment within a task it not allowed
   // in Verilog, but it is allowed in SystemVerilog.
   task foo (input integer x, output integer y);
      integer step = 3;
      y = x + step;
   endtask // foo

   integer	  a, b;
   initial begin
      a = 3;
      foo(a, b);
      if (b !== 6) begin
	 $display("FAILED");
	 $finish;
      end
      $display("PASSED");
   end

endmodule // main
