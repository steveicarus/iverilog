/*
 * This demonstrates that strings can be used as
 * constructed formats in $display et al.
 */
module main;

   string foo;
   string bar;

   initial begin
      bar = "PAS";
      foo = {bar, "SED"};
      if (foo != "PASSED") begin
	 $display("FAILED (1)");
	 $finish;
      end

      foo = "PAS";
      bar = "SED";

      $display({foo,bar});
      $finish;
   end
endmodule // main
