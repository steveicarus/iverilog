// pr1697250

module test();

   wire active;
   reg [63:0] bus;

   assign     active = ((|(bus)===0)?0:1);

   initial begin
      bus = 'haaaa;
      #1 if (active !== 1) begin
	 $display("FAILED -- bus=%h, active=%b", bus, active);
	 $finish;
      end

      bus = 0;
      #1 if (active !== 0) begin
	 $display("FAILED == bus=%h, active=%b", bus, active);
	 $finish;
      end

      $display("PASSED");
   end
endmodule
