/*
 * Based on bug report PR#856
 */

module foo5 ();

parameter a = 3;
parameter b = 5;

localparam c = a ^ b;

initial begin
   $display("%b = %b ^ %b", c, a, b);
   if (c !== 6) begin
      $display("FAILED -- c = %b", c);
      $finish;
   end

   $display("PASSED");
end

endmodule // foo5
