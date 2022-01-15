module extension_bug();

reg       x;
reg [3:0] a, b;

initial begin
   x = 1'b1;

   a = ~1'b1;
   b = ~x;

   $display("a = %b", a);
   if (a !== 4'b1110) begin
      $display("FAILED");
      $finish;
   end
   $display("b = %b", b);
   if (b !== 4'b1110) begin
      $display("FAILED");
      $finish;
   end

   $display("PASSED");
   $finish;
end

endmodule
