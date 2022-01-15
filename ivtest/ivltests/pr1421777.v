/*
 * This is the essence of tracker id#1421777. The problem is the error
 * message around the "... dut.tmp" expression. This probram won't
 * compile while the reported bug still lives.
 */
module main;

   reg b;
   wire a;
   // The a.tmp is valid, but tricky because it is an implicit wire.
   wire foo = dut.tmp;
   X dut(a, b);

   initial begin
      b = 0;
      #1 $display("a=%b, tmp=%b", a, foo);
      if (a !== foo) begin
	 $display("FAILED");
	 $finish;
      end
      $display("PASSED");
   end

endmodule // main

module X(output a, input b);

   not (tmp, b);
   buf(a, tmp);

endmodule // X
