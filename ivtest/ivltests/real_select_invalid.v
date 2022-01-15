module top;
  parameter real rpar = 2.0;
  real rvar;
  real rarr [1:0];
  real rout, rtmp;
  wire real wrarr [1:0];
  wire real wrbslv, wrpslv, wruplv, wrdolv;
  wire real wrbstr, wrpstr, wruptr, wrdotr;

  wire real wrpbs = rpar[0];
  wire real wrpps = rpar[0:0];
  wire real wrpup = rpar[0+:1];
  wire real wrpdo = rpar[0-:1];

  wire real wrbs = rvar[0];
  wire real wrps = rvar[0:0];
  wire real wrup = rvar[0+:1];
  wire real wrdo = rvar[0-:1];

  wire real wrabs = rarr[0][0];
  wire real wraps = rarr[0][0:0];
  wire real wraup = rarr[0][0+:1];
  wire real wrado = rarr[0][0-:1];

  assign wrbslv[0] = rvar;
  assign wrpslv[0:0] = rvar;
  assign wruplv[0+:1] = rvar;
  assign wrdolv[0-:1] = rvar;

  assign wrarr[0][0] = rvar;
  assign wrarr[0][0:0] = rvar;
  assign wrarr[0][0+:1] = rvar;
  assign wrarr[0][0-:1] = rvar;

  tran(wrbstr[0], wrarr[1]);
  tran(wrpstr[0:0], wrarr[1]);
  tran(wruptr[0+:1], wrarr[1]);
  tran(wrdotr[0-:1], wrarr[1]);

  submod1 s1 (wrbstr[0], wrpstr[0:0], wruptr[0+:1], wrdotr[0-:1]);
  submod2 s2 (wrbstr[0], wrpstr[0:0], wruptr[0+:1], wrdotr[0-:1]);
  submod3 s3 (wrbstr[0], wrpstr[0:0], wruptr[0+:1], wrdotr[0-:1]);

  initial begin
    rtmp = rpar[0];
    rtmp = rpar[0:0];
    rtmp = rpar[0+:1];
    rtmp = rpar[0-:1];

    rtmp = rvar[0];
    rtmp = rvar[0:0];
    rtmp = rvar[0+:1];
    rtmp = rvar[0-:1];

    rtmp = rarr[0][0];
    rtmp = rarr[0][0:0];
    rtmp = rarr[0][0+:1];
    rtmp = rarr[0][0-:1];

    rout[0] = 2.0;
    rout[0:0] = 2.0;
    rout[0+:1] = 2.0;
    rout[0-:1] = 2.0;

    rarr[0][0] = 1.0;
    rarr[0][0:0] = 1.0;
    rarr[0][0+:1] = 1.0;
    rarr[0][0-:1] = 1.0;
  end
endmodule

module submod1(arg1, arg2, arg3, arg4);
  input arg1, arg2, arg3, arg4;
  wire real arg1, arg2, arg3, arg4;

  initial $display("In submod1 with %g, %g, %g, %g", arg1, arg2, arg3, arg4);
endmodule

module submod2(arg1, arg2, arg3, arg4);
  output arg1, arg2, arg3, arg4;
  real arg1, arg2, arg3, arg4;

  initial $display("In submod2 with %g, %g, %g, %g", arg1, arg2, arg3, arg4);
endmodule

module submod3(arg1, arg2, arg3, arg4);
  inout arg1, arg2, arg3, arg4;
  wire real arg1, arg2, arg3, arg4;

  initial $display("In submod3 with %g, %g, %g, %g", arg1, arg2, arg3, arg4);
endmodule
