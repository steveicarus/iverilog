module test(w);
   input wire w;
   wire a, b;


   initial begin
      #11 $finish;
   end

   assign b = 0;

   assign a = !w | b;

   always @(a) begin
      $display($time, ": Wire a is now ", a);
   end
endmodule
