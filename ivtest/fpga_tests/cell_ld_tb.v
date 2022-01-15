`timescale  100 ps / 10 ps

module main;

   wire Q;
   reg	D, G;

   LD u1 (.Q(Q), .D(D), .G(G));

   initial begin
      D = 0;
      G = 1;
      #1 if (Q !== 0) begin
	 $display("FAILED -- D=%b, G=%b --> Q=%b", D, G, Q);
	 $finish;
      end

      D = 1;
      #1 if (Q !== 1) begin
	 $display("FAILED -- D=%b, G=%b --> Q=%b", D, G, Q);
	 $finish;
      end

      G = 0;
      #1 if (Q !== 1) begin
	 $display("FAILED -- D=%b, G=%b --> Q=%b", D, G, Q);
	 $finish;
      end

      D = 0;
      #1 if (Q !== 1) begin
	 $display("FAILED -- D=%b, G=%b --> Q=%b", D, G, Q);
	 $finish;
      end

      G = 1;
      #1 if (Q !== 0) begin
	 $display("FAILED -- D=%b, G=%b --> Q=%b", D, G, Q);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
