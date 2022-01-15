// test that .* implicit ports work with override
module m(input a, output b, output c, output d, output e);
assign b = a;
assign c = ~a;
assign d = ~a;
assign e = ~a;
endmodule

module top;
reg a;
reg x;
wire b, d;
m foo(.a(x), .e(), .*, .c(d), .d());
m foo2(.a(x), .d(), .*, .c(), .e());
m foo3(.a(x), .*, .d(), .c(), .e());
m foo4(.*, .a(x), .d(), .c(), .e());
m foo5(.a(x), .d(), .c(), .*, .e());
m foo6(.a(x), .d(), .c(), .e(), .*);

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
