// This should generate an error

module main;

   reg[7:0] shared;
   wire [7:0] not_shared = ~shared;

program test1;
   initial shared <= 'h55;
endprogram :test1

program test2;
   reg [7:0]  tmp;
   final begin
      if (shared !== 'h55) begin
	 $display("FAILED -- shared=%b is not correct", shared);
	 $finish;
      end

      tmp <= ~shared; // ERROR: only blocking assign in final block
      if (not_shared !== 'haa) begin
	 $display("FAILED -- not_shared is not correct", not_shared);
	 $finish;
      end

      $display("PASSED");
   end

endprogram :test2

endmodule // main
