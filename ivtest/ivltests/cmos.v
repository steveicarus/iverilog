module top;
  reg nctl, pctl, b;
  wire a, c;
  initial begin
    $monitor(a,c,,"%v",a,,"%v",c,,b,,nctl,,pctl);
    b = 0;
    nctl = 0;
    pctl = 1;
    #1 nctl = 1; pctl = 0;
    #1 nctl = 1; pctl = 1;
    #1 nctl = 0; pctl = 0;
    #1 nctl = 1'bx; pctl = 0;
    #1 nctl = 1; pctl = 1'bx;
    #1 nctl = 1'bx; pctl = 1;
    #1 nctl = 0; pctl = 1'bx;
    #1 nctl = 1'bx; pctl = 1'bx;

    #1 b = 1; nctl = 0; pctl = 1;
    #1 nctl = 1; pctl = 0;
    #1 nctl = 1; pctl = 1;
    #1 nctl = 0; pctl = 0;
    #1 nctl = 1'bx; pctl = 0;
    #1 nctl = 1; pctl = 1'bx;
    #1 nctl = 1'bx; pctl = 1;
    #1 nctl = 0; pctl = 1'bx;
    #1 nctl = 1'bx; pctl = 1'bx;

    #1 b = 1'bx; nctl = 0; pctl = 1;
    #1 b = 1'bx; nctl = 1; pctl = 0;
    #1 b = 1'bx; nctl = 1'bx; pctl = 1'bx;

    #1 b = 1'bz; nctl = 0; pctl = 1;
    #1 b = 1'bz; nctl = 1; pctl = 0;
    #1 b = 1'bz; nctl = 1'bx; pctl = 1'bx;
  end

  nmos n1 (a, b, nctl);
  pmos p1 (a, b, pctl);
  cmos c1 (c, b, nctl, pctl);
endmodule
