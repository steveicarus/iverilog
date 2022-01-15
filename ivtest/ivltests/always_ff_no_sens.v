module test;
   logic y;

   always_ff begin
      y = 1'b0;
   end

  initial $display("FAILED");
endmodule
