module top;
  parameter pval = 7;
  string sval, strvm, strcm, strvl, strcl;
  real ridx;
  integer in;
  reg cavm, cacm, cavl, cacl;
  reg vvm, vcm, vvl, vcl;
  reg pvm, pcm, pvl, pcl;
  reg svm, scm, svl, scl;
  integer calvm, calcm, calvl, calcl;
  integer vlvm, vlcm, vlvl, vlcl;

  assign cavm = in[ridx:1];
  assign cacm = in[0.5:1];
  assign cavl = in[1:ridx];
  assign cacl = in[1:0.5];
  assign calvm[ridx:1] = 1'b1;
  assign calcm[0.5:1] = 1'b1;
  assign calvl[1:ridx] = 1'b1;
  assign calcl[1:0.5] = 1'b1;

  initial begin
    in = 7;
    ridx = 0.5;
    sval = "ABC";
    vvm = in[ridx:1];
    vcm = in[0.5:1];
    vvl = in[1:ridx];
    vcl = in[1:0.5];
    vlvm[ridx:1] = 1'b1;
    vlcm[0.5:1] = 1'b1;
    vlvl[1:ridx] = 1'b1;
    vlcl[1:0.5] = 1'b1;
    pvm = pval[ridx:1];
    pcm = pval[0.5:1];
    pvl = pval[1:ridx];
    pcl = pval[1:0.5];
    svm = sval[ridx:1];
    scm = sval[0.5:1];
    svl = sval[1:ridx];
    scl = sval[1:0.5];
    strvm[ridx:1] = "a";
    strcm[0.5:1] = "a";
    strvl[1:ridx] = "a";
    strcl[1:0.5] = "a";
  end
endmodule
