module Main();
   logic[4:0] r5;

   initial begin
      r5 = 5'(3'd7 + 3'd6);
      $display("r5 = %b, %d", r5, r5);

      if (r5 === 5'b01101)
         $display("PASSED");
      else
         $display("FAILED");
   end

endmodule
