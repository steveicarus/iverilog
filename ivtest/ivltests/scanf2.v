`timescale 1ns / 100ps

module main;

   real x;
   integer rc;
   time    x_int;

   initial begin
      rc = $sscanf("8.125", "%f", x);

      if (x != 8.125) begin
	 $display("FAILED -- x=%f", x);
	 $finish;
      end

      if (rc !== 1) begin
	 $display("FAILED -- rc = %d", rc);
	 $finish;
      end

      $timeformat(-3,2,"ms",10);
      rc = $sscanf("8.125", "%t", x);

      x_int = x;
      if (x_int != 8130000) begin
	 $display("FAILED -- t=%f (%h)!=(%h)", x,
		  $realtobits(x), $realtobits(8130000.0));
	 $finish;
      end

      if (rc !== 1) begin
	 $display("FAILED -- rc = %d", rc);
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
