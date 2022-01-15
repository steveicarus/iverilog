module main;

   wire struct packed {
      logic m1;
      logic [7:0] m8;
   } foo;

   assign foo = {1'b1, 8'ha5};

   struct packed {
      logic [3:0] m4;
      logic [7:0] m8;
   } bar;

   initial begin
      #1 /* wait for logic to settle. */;
      bar.m8 <= foo.m8[7:0];
      bar.m4 <= foo.m8[7:4];
      #1 $display("bar8=%h, bar4=%h", bar.m8, bar.m4);
      if (bar.m8 !== 8'ha5) begin
	 $display("FAILED");
	 $finish;
      end
      if (bar.m4 !== 4'ha) begin
	 $display("FAILED");
	 $finish;
      end
      $display("PASSED");
   end

endmodule // main
