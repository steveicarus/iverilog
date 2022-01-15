module main;

   real rval;

   wire [63:0] wbits = $realtobits(rval);
   reg [63:0]  rbits;

   initial begin
      rval = 1.5;
      rbits = $realtobits(rval);

      #1 /* Let the wbits value propagate */ ;

      if (rbits !== 64'h3ff80000_00000000) begin
	 $display("FAILED -- rbits=%h", rbits);
	 $finish;
      end

      if (wbits !== rbits) begin
	 $display("FAILED -- rval=%f, rbits=%h, wbits=%h", rval, rbits, wbits);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
