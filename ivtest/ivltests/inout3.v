module main;

   wire qh = 1'bz;
   wire [1:0] Q;
   reg [2:0] D;

   buft a({qh,Q}, D);

   reg	      x;
   //assign     D[0] = x;

   initial begin
      D = 3'bzz0;
      #1 $display("Q=%b, D=%b", Q, D);
      if (Q !== 2'bz0) begin
	 $display("FAILED");
	 $finish;
      end

      D[0] = 1;
      #1 $display("Q=%b, D=%b", Q, D);
      if (Q !== 2'bz1) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main

module buft(inout [2:0] T, input [2:0] D);

   assign T = D;

endmodule // buft
