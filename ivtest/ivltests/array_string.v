
module main;

   localparam AMAX = 3;
   string foo [0:AMAX];

   localparam STRIDE = 2;
   string source = "a0b1c2d3";

   int	  idx;
   initial begin
      for (idx = 0 ; idx <= AMAX ; idx = idx+1) begin
	 foo[idx] = source.substr(idx*STRIDE+0, idx*STRIDE+STRIDE-1);
	 $display("foo[%0d] = %0s", idx, foo[idx]);
      end

      if (foo[2] != "c2") begin
	 $display("FAILED -- foo[2] = %0s", foo[2]);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
