module top;
  reg [47:0] out1, out2, out3, out4, out5;
  integer i;

  initial begin
    for (i=-1 ; i<2; i=i+1) begin
      // i is signed so should it be sign extended?
      out1 = 48'd16 + i;
      // I would have expected this to be the same as (i+0) below!
      out2 = 48'd16 + (i);
      // All the rest of these are sign extended?
      out3 = 48'd16 + (i+0);
      out4 = 48'sd16 + i;
      out5 = 48'd16 + (i-1);
      $display("16 + %2d = %10d, %10d, %2d, %2d,  -1 = %2d",
               i, out1, out2, out3, out4, out5);
    end
  end
endmodule
