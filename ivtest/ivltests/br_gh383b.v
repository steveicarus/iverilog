module test;
   bit [7:0] i, x[], y[], z[];


  initial begin
    x = new [4];

    for (i = 0; i < 4; i = i + 1) x[i] = 1 + i;
    y = x;
    z = new [4](x);
    for (i = 0; i < 4; i = i + 1) y[i] = 4 - i;
    for (i = 0; i < 4; i = i + 1) z[i] = 8 - i;
     // Expected output:
     // 1   2   3   4
     // 4   3   2   1
     // 8   7   6   5
    $display(x[0],,x[1],,x[2],,x[3]);
    $display(y[0],,y[1],,y[2],,y[3]);
    $display(z[0],,z[1],,z[2],,z[3]);
  end

endmodule
