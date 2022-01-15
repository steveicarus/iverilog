 module math(a, b, c, z);
 input signed [19:0] a, b;
 input signed [24:0] c;
 output signed [24:0] z;

 assign z = a + b + c - (c >>> 1);
 endmodule

 module test();
 reg signed [19:0] a, b;
 reg signed [24:0] z, c;

 wire signed [24:0] y;

 wire signed [24:0] w = a + b + c - (c >>> 1);

 math m(a,b,c,y);

 initial begin
 a = -5;
 $display("a = %x %d", a, a);
 b = 0;
 $display("b = %x %d", b, b);
 c = 8;
 $display("c = %x %d", c, c);
 z = a + b + c - (c >>> 1);
    #1 /* delay for things to settle. */;
 $display("z = %x, %d, %b", z, z, z);
 $display("y = %x, %d, %b", y, y, y);
 $display("w = %x, %d, %b", w, w, w);
 $display("%b %b %b %b", $is_signed(a),
 $is_signed(b), $is_signed(c), $is_signed(c >>> 1));
 end
 endmodule
