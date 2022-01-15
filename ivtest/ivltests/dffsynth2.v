/*
 * This program tests the synthesis of small memories, including
 * aysnchronous read w/ synchronous write.
 */
module main;

   reg clk;

   reg Q, D;
   (* ivl_synthesys_on *)
   always @(negedge clk)
     Q <= D;

   (* ivl_synthesys_off *)
   initial begin
      clk = 1;
      D = 0;
      #2 clk = 0;
      #2 clk = 1;
      #2 if (Q !== 0) begin
	 $display("FAILED -- initial setup D=%b, Q=%b", D, Q);
	 $finish;
      end

      D = 1;
      #2 clk = 0;

      #2 if (Q !== 1) begin
	 $display("FAILED -- negedge clk failed D=%b, Q=%b", D, Q);
	 $finish;
      end

      D = 0;
      #2 clk = 1;
      #2 if (Q !== 1) begin
	 $display("FAILED -- posedge clk tripped FF. D=%b, Q=%b", D, Q);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
