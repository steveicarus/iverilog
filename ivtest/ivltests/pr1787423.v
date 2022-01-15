module top;
  reg in;
  wire bf1, bf2, nt1, nt2, pd1, pd2, pu1, pu2;

  initial begin
    $monitor(bf1, bf2,, nt1, nt2,, pd1, pd2,, pu1, pu2,, in);
    in = 0;

    #1 in = 1;
    #1 in = 0;
  end

  buf (bf1, bf2, in);
  not (nt1, nt2, in);
  pulldown (pd1, pd2);
  pullup (pu1, pu2);
endmodule
