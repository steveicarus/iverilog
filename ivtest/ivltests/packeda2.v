module main;

   wire logic [3:0][7:0] foo;

   genvar idx;
   for (idx = 0 ; idx <= 3 ; idx = idx+1) begin: test
      test dut (.sum(foo[idx]), .a(idx));
   end

   logic [7:0] tmp;
   initial begin
      #0; // avoid time-zero race
      for (tmp = 0 ; tmp <= 3 ; tmp = tmp+1) begin
	 //if ($bits(foo[tmp]) !== 8) begin
	 //   $display("FAILED -- $bits = %d", $bits(foo[tmp]));
	 //   $finish;
	 //end
	 if (foo[tmp] !== (tmp+8'd5)) begin
	    $display("FAILED -- foo[%d] = %b", tmp, foo[tmp]);
	    $finish;
	 end
      end
      $display("PASSED");
   end

endmodule // main

module test (output logic[7:0] sum, input logic [7:0]a);

   assign sum = a + 8'd5;

endmodule // test
