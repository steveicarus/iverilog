/*
 * This program demonstrates non-constant part selects
 * applied to a parameter value.
 */
module main;

   parameter foo = 32'h76543210;

   reg [3:0] tmp;
   reg [3:0] idx;

   initial begin
      if (foo[0 +: 4] !== 4'h0) begin
	    $display("FAILED -- %b !== 0", foo[0 +: 4]);
	    $finish;
      end

      if (foo[4 +: 4] !== 4'h1) begin
	    $display("FAILED -- %b !== 1", foo[4 +: 4]);
	    $finish;
      end

      if (foo[8 +: 4] !== 4'h2) begin
	    $display("FAILED -- %b !== 2", foo[8 +: 4]);
	    $finish;
      end

      if (foo[12+: 4] !== 4'h3) begin
	    $display("FAILED -- %b !== 3", foo[12 +: 4]);
	    $finish;
      end

      for (idx = 0 ;  idx < 8 ;  idx = idx + 1) begin
	 tmp = foo[(idx*4) +: 4];
	 if (tmp !== idx) begin
	    $display("FAILED -- %b !== %b", idx, tmp);
	    $finish;
	 end

      end

      for (idx = 0 ;  idx < 8 ;  idx = idx + 1) begin
	 tmp = foo[(idx*4+3) -: 4];
	 if (tmp !== idx) begin
	    $display("FAILED -- %b !== %b", idx, tmp);
	    $finish;
	 end

      end

      $display("PASSED");
   end

endmodule // main
