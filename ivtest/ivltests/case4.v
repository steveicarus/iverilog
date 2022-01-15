/*
 * This tests the synthesis of a very sparse case statement. The
 * combinational case statement below specifies only two of 256
 * possible selections, with all the remaining left to the default.
 * What's more, all the inputs to the MUX are constant, giving
 * even further opportunity for optimization.
 */
module main;

   reg [7:0] val;
   reg [7:0] out;

   (* ivl_combinational *)
   always @ (val) begin
        case (val)
            8'h2a: out = 8'h40 ;
            8'h1f: out = 8'h20 ;
            default: out = 8'h04 ;
        endcase
   end

   integer idx;
   (* ivl_synthesis_off *) initial begin
      for (idx = 0 ;  idx < 256 ;  idx = idx + 1) begin
	 val <= idx;
	 #1 ;
	 if (val == 8'h2a) begin
	    if (out !== 8'h40) begin
	       $display("FAILED -- val=%h, out=%h (%b)", val, out, out);
	       $finish;
	    end
	 end else if (val == 8'h1f) begin
	    if (out !== 8'h20) begin
	       $display("FAILED -- val=%h, out=%h (%b)", val, out, out);
	       $finish;
	    end
	 end else if (out !== 8'h04) begin
	    $display("FAILED -- val=%h, out=%h (%b)", val, out, out);
	    $finish;
	 end
      end
      $display("PASSED");
   end

endmodule // main
