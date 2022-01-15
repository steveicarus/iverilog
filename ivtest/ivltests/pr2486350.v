`timescale 1 s / 1 fs

module main;

   reg a;

   initial begin
      a = 1'b0;
      #10 ;
      $display("simtime=%d (%h)", $simtime, $simtime);
      a = 1'b1;
   end

   initial begin
      #100 $finish(0);
   end

endmodule // main
