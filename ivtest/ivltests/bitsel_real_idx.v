module top;
  parameter pval = 7;
  string sval, strv, strc;
  real ridx;
  integer in;
  reg cav, cac;
  reg vv, vc, pv, pc, sv, sc;
  integer calv, calc;
  integer vlv, vlc;

  assign cav = in[ridx];
  assign cac = in[0.5];
  assign calv[ridx] = 1'b1;
  assign calc[0.5] = 1'b1;

  initial begin
    in = 7;
    sval = "ABC";
    ridx = 0.5;
    vv = in[ridx];
    vc = in[0.5];
    vlv[ridx] = 1'b1;
    vlc[0.5] = 1'b1;
    pv = pval[ridx];
    pc = pval[0.5];
    sv = sval[ridx];
    sc = sval[0.5];
    strv[ridx] = "a";
    strc[0.5] = "a";
  end
endmodule
