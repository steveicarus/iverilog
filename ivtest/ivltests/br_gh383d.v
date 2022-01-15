module test;
   real x[], y[], z[];
   real src[0:7];
   int	  i;

   initial begin
      src[0] = 1.0;
      src[1] = 2.0;
      src[2] = 3.0;
      src[3] = 4.0;
      src[4] = 5.0;
      src[5] = 6.0;
      src[6] = 7.0;
      src[7] = 8.0;

      x = new [4];

      for (i = 0; i < 4; i = i + 1) x[i] = src[i];

      y = x;
      z = new [4](x);
      for (i = 0; i < 4; i = i + 1) y[i] = src[3 - i];
      for (i = 0; i < 4; i = i + 1) z[i] = src[7 - i];
      // Expected output:
      // 1.00000 2.00000 3.00000 4.00000
      // 4.00000 3.00000 2.00000 1.00000
      // 8.00000 7.00000 6.00000 5.00000
      $display(x[0],,x[1],,x[2],,x[3]);
      $display(y[0],,y[1],,y[2],,y[3]);
      $display(z[0],,z[1],,z[2],,z[3]);
   end

endmodule
