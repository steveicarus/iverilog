module x;

   parameter bar0_low = 5'd16; // Register Space
   reg [31:bar0_low] base_address0;

   reg [31:0]	     ad_in_d;
   wire [0:5]	     hit_bar;

   wire		     a = |bar0_low;
   wire [31:0]	     e = ad_in_d[31:bar0_low];
   wire		     b = (base_address0==e);
   wire		     d = b;

   assign	     hit_bar[0] = a ? d : 0;

   initial begin
      if ($bits(base_address0) != 16) begin
	 $display("FAILED -- $bits(base_address0) = %0d", $bits(base_address0));
	 $finish;
      end

      $display("PASSED");
   end

endmodule
