module bar;
   reg [24:1] original = 24'h123456;
   reg [8:1]  second, minus_indexed, plus_indexed;

   integer    tmp;
   initial begin
      second = original[16:9];
      minus_indexed = original[16 -:8];
      plus_indexed = original[9 +:8];
      $display ("Orig = %h, Second = %h, Minus Indexed = %h, Plus Indexed = %h",
		original, second, minus_indexed, plus_indexed);

      tmp = 9;
      second = original[16:9];
      minus_indexed = original[tmp+7 -:8];
      plus_indexed = original[tmp +:8];
      $display ("Orig = %h, Second = %h, Minus Indexed = %h, Plus Indexed = %h",
		original, second, minus_indexed, plus_indexed);
   end
 endmodule
