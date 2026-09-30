// Mixed scalar/vector gate array outputs are legal but not yet supported.
module top;
  wire [3:0] wide;
  wire narrow, in;
  buf b0[3:0](narrow, wide, in);
  buf b1[3:0](wide, narrow, in);
  not n0[3:0](narrow, wide, in);
  not n1[3:0](wide, narrow, in);
  tran t0[3:0](narrow, wide);
  tran t1[3:0](wide, narrow);
endmodule
