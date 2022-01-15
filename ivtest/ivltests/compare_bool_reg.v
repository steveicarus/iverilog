module main;

   reg bool [31:0] idx;

   initial begin
      idx = 0;
      if (idx < 17) $display("PASSED");
      else $display("FAILED");
   end

endmodule
