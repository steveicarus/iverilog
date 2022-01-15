`timescale 1ns / 100ps

module main;

   integer rc;
   reg [7:0] x, y;
   reg [23:0] z;

   initial begin
      rc = $sscanf("a b cd e", "%c %c %s", x, y, z);

      if (rc !== 3) begin
	 $display("FAILED -- rc = %d", rc);
	 $finish;
      end

      if (x != 'h61) begin
	 $display("FAILED -- x=%h", x);
	 $finish;
      end

      if (y != 'h62) begin
	 $display("FAILED -- y=%h", y);
	 $finish;
      end

      if (z !== 24'h00_63_64) begin
	 $display("FAILED == z=%h", z);
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
