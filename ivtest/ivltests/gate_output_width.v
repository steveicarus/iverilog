// Different output widths must produce diagnostics, not an assertion failure.
module top;
  wire [3:0] wide;
  wire narrow, in;
  buf b0(narrow, wide, in);
  buf b1(wide, narrow, in);
  not n0(narrow, wide, in);
  not n1(wide, narrow, in);
  tran t0(narrow, wide);
  tran t1(wide, narrow);
endmodule
