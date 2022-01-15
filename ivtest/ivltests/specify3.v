// specify3.v

module top;
   reg a, b, fast;
   wire q;

   initial begin
      a = 0;
      b = 0;
      fast = 0;
      #10 $monitor($time,,"a=%b, b=%b, fast=%b, q=%b", a, b, fast, q);
      #10 a = 1;
      #10 a = 0;
      #10 b = 1;
      #10 b = 0;
      #10 a = 1;
      #10 fast = 1;
      #10 b = 1;
      #10 b = 0;
      #10 a = 0;
      #10 a = 1;

      #10 $finish(0);
   end

   myxor g1 (q, a, b, fast);

endmodule

module myxor (q, a, b, fast);
   output q;
   input  a, b, fast;

   xor g1 (q, a, b);

   specify
      if (fast) (b => q) = 1;
      if (fast) (a => q) = 1;
      if (~fast) (b => q) = 4;
      if (~fast) (a => q) = 4;
   endspecify
endmodule
