module test;
   string x[], y[], z[];
   string src[0:7];
   int	  i;

   initial begin
      src[0] = "a";
      src[1] = "b";
      src[2] = "c";
      src[3] = "d";
      src[4] = "e";
      src[5] = "f";
      src[6] = "g";
      src[7] = "h";

      x = new [4];

      for (i = 0; i < 4; i = i + 1) x[i] = src[i];

      y = x;
      z = new [4](x);
      for (i = 0; i < 4; i = i + 1) y[i] = src[3 - i];
      for (i = 0; i < 4; i = i + 1) z[i] = src[7 - i];
     // Expected output:
     // a   b   c   d
     // d   c   b   a
     // h   g   f   e
      $display(x[0],,x[1],,x[2],,x[3]);
      $display(y[0],,y[1],,y[2],,y[3]);
      $display(z[0],,z[1],,z[2],,z[3]);
   end

endmodule
