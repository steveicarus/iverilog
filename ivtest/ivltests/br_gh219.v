module Main();

   logic[2:0] a = 3'b111;
   logic signed[2:0] a_signed = 3'b111;

   logic[2:0] b = 0;

   logic[3:0] c0;
   logic[3:0] c1;

   initial begin
      c0 = 4'($signed(a)) + b;
      c1 = 4'(a_signed) + b;

      $display("c0: %b", c0);
      $display("c1: %b", c1);

      if (c0 === 4'b1111 && c1 === 4'b1111)
         $display("PASSED");
      else
         $display("FAILED");
   end

endmodule
