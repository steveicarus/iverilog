
module main;

   wire [4:0] foo;
   struct packed {
      logic [3:0] bar4;
      logic [3:0] bar0;
   } bar;

   assign foo = bar.bar0;

   initial begin
      bar = 'h5a;
      #1 if (bar.bar0 !== 4'ha || bar.bar4 != 4'h5) begin
	 $display("FAILED -- bar.bar0=%b, bar.bar4=%b", bar.bar0, bar.bar4);
	 $finish;
      end

      if (foo !== 5'h0a) begin
	 $display("FAILED -- foo=%b", foo);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
