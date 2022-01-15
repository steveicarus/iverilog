`begin_keywords "1364-2005"
module top;
  reg svar;
  reg sarr [1:0];
  reg sout, stmp;
  wire wsarr [1:0];
  wire wsbslv, wspslv, wsuplv, wsdolv;
  wire wsbstr, wspstr, wsuptr, wsdotr;

  wire wsbs = svar[0];
  wire wsps = svar[0:0];
  wire wsup = svar[0+:1];
  wire wsdo = svar[0-:1];

  wire wsabs = sarr[0][0];
  wire wsaps = sarr[0][0:0];
  wire wsaup = sarr[0][0+:1];
  wire wsado = sarr[0][0-:1];

  assign wsbslv[0] = svar;
  assign wspslv[0:0] = svar;
  assign wsuplv[0+:1] = svar;
  assign wsdolv[0-:1] = svar;

  assign wsarr[0][0] = svar;
  assign wsarr[0][0:0] = svar;
  assign wsarr[0][0+:1] = svar;
  assign wsarr[0][0-:1] = svar;

  tran(wsbstr[0], wsarr[1]);
  tran(wspstr[0:0], wsarr[1]);
  tran(wsuptr[0+:1], wsarr[1]);
  tran(wsdotr[0-:1], wsarr[1]);

  submod1 s1 (wsbstr[0], wspstr[0:0], wsuptr[0+:1], wsdotr[0-:1]);
  submod2 s2 (wsbstr[0], wspstr[0:0], wsuptr[0+:1], wsdotr[0-:1]);
  submod3 s3 (wsbstr[0], wspstr[0:0], wsuptr[0+:1], wsdotr[0-:1]);

  task stask;
    input a;
    reg local;
    begin
      local = a[0];
      local = a[0:0];
      local = a[0+:1];
      local = a[0-:1];
    end
  endtask

  initial begin
    stmp = svar[0];
    stmp = svar[0:0];
    stmp = svar[0+:1];
    stmp = svar[0-:1];

    stmp = sarr[0][0];
    stmp = sarr[0][0:0];
    stmp = sarr[0][0+:1];
    stmp = sarr[0][0-:1];

    sout[0] = 1'b0;
    sout[0:0] = 1'b0;
    sout[0+:1] = 1'b0;
    sout[0-:1] = 1'b0;

    sarr[0][0] = 1'b0;
    sarr[0][0:0] = 1'b0;
    sarr[0][0+:1] = 1'b0;
    sarr[0][0-:1] = 1'b0;
  end
endmodule

module submod1(arg1, arg2, arg3, arg4);
  input arg1, arg2, arg3, arg4;
  wire arg1, arg2, arg3, arg4;

  initial $display("In submod1 with %b, %b, %b, %b", arg1, arg2, arg3, arg4);
endmodule

module submod2(arg1, arg2, arg3, arg4);
  output arg1, arg2, arg3, arg4;
  wire arg1, arg2, arg3, arg4;

  initial $display("In submod2 with %b, %b, %b, %b", arg1, arg2, arg3, arg4);
endmodule

module submod3(arg1, arg2, arg3, arg4);
  inout arg1, arg2, arg3, arg4;
  wire arg1, arg2, arg3, arg4;

  initial $display("In submod3 with %b, %b, %b, %b", arg1, arg2, arg3, arg4);
endmodule
`end_keywords
