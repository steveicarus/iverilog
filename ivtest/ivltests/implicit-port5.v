// test that .* implicit ports work with override
module m(input a, output b, output c);
assign b = a;
assign c = ~a;
endmodule

module top;
reg a;
reg x;
wire b, d;
m foo(.a(x), .*, .c(d));

initial begin
   a = 0;
   x = 1;
   #1 if (b !== x || d !== ~x) begin
      $display("FAILED -- a=%b, x=%b, b=%b, d=%b", a, x, b, d);
   end
   #1 a = 1;
   #1 if (b !== x || d !== ~x) begin
      $display("FAILED -- a=%b, x=%b, b=%b, d=%b", a, x, b, d);
   end
   $display("PASSED");
end
endmodule
