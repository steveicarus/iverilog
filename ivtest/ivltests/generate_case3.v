module main;

   parameter COND = 1;
   parameter SEL = 0;
   parameter VAL0 = 0;
   parameter VAL1 = 1;
   parameter VAL2 = 2;

   wire [3:0] foo;
   if (COND) begin
     case (SEL)
       0: assign foo = VAL0;
       1: assign foo = VAL1;
       2: assign foo = VAL2;
     endcase // case (SEL)
   end else begin
      assign foo = 'bx;
   end

   initial begin
      #1 $display("foo = %b", foo);
      if (foo !== VAL0) begin
	 $display("FAILED");
	 $finish;
      end
      $display("PASSED");
      $finish;
   end

endmodule // main
