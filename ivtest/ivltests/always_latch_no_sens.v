module test;
   logic y;

   always_latch begin
      y = 1'b0;
   end

  initial $display("FAILED");
endmodule
