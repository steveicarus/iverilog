module main;
   parameter WORD_WID = 3;
   parameter WORD_CNT = 8;

   reg [WORD_WID-1: 0] mem [0:WORD_CNT-1], tmp;

   integer	       idx, jdx;
   initial begin
      for (idx = 0 ;  idx < WORD_CNT ;  idx = idx+1)
	mem[idx] = idx;

      for (idx = 0 ;  idx < WORD_CNT ;  idx = idx+1) begin
	 tmp = idx;

	 if (mem[idx][2:1] !== tmp[2:1]) begin
	    $display("FAILED -= mem[%d][2:1]=%b, tmp[2:1]=%b",
		     idx, mem[idx][2:1], tmp[2:1]);
	    $finish;
	 end

	 if (mem[idx][1:0] !== tmp[1:0]) begin
	    $display("FAILED -= mem[%d][1:0]=%b, tmp[1:0]=%b",
		     idx, mem[idx][1:0], tmp[1:0]);
	    $finish;
	 end

	 for (jdx = 0 ;  jdx < WORD_WID ;  jdx = jdx+1)
	   if (mem[idx][jdx +:2] !== tmp[jdx +:2]) begin
	      $display("FAILED -- mem[%d][%d +:2]=%b, tmp[%d +:2]=%b",
		       idx, jdx, mem[idx][jdx+:2], jdx, tmp[jdx+:2]);
	      $finish;
	   end
      end

      $display("PASSED");
   end
endmodule // main
