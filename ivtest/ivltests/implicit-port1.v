// test basic implicit ports work
module m(input a, output b, output c);
assign b = a;
assign c = ~a;
endmodule

module top;
reg a;
wire b, d;
m foo(.a, .b, .c(d));

initial begin
   a = 0;
   #1 if (b !== a || d !== ~a) begin
      $display("FAILED -- a=%b, b=%b, d=%b", a, b, d);
   end
   #1 a = 1;
   #1 if (b !== a || d !== ~a) begin
      $display("FAILED -- a=%b, b=%b, d=%b", a, b, d);
   end
   $display("PASSED");
end
endmodule
