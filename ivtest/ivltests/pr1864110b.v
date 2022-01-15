module top;
  wire real minus;
  real in;

  assign minus = -in; // Should be arith/sub.r Cr<0>, <net>

  initial begin
    $monitor(minus,, in);

       in = 3.0;
    #1 in = 4.0;
    #1 in = 6.0;
  end
endmodule
