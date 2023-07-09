module top;
  parameter pval = 7;
  string sval, strvb, strcb, strvw, strcw;
  real ridx;
  integer in;
  reg cavb, cacb, cavw, cacw;
  reg vvb, vcb, vvw, vcw;
  reg pvb, pcb, pvw, pcw;
  reg svb, scb, svw, scw;
  integer calvb, calcb, calvw, calcw;
  integer vlvb, vlcb, vlvw, vlcw;

  assign cavb = in[ridx-:1];
  assign cacb = in[0.5-:1];
  assign cavw = in[1-:ridx];
  assign cacw = in[1-:0.5];
  assign calvb[ridx-:1] = 1'b1;
  assign calcb[0.5-:1] = 1'b1;
  assign calvw[1-:ridx] = 1'b1;
  assign calcw[1-:0.5] = 1'b1;

  initial begin
    in = 7;
    ridx = 0.5;
    sval = "ABC";
    vvb = in[ridx-:1];
    vcb = in[0.5-:1];
    vvw = in[1-:ridx];
    vcw = in[1-:0.5];
    vlvb[ridx-:1] = 1'b1;
    vlcb[0.5-:1] = 1'b1;
    vlvw[1-:ridx] = 1'b1;
    vlcw[1-:0.5] = 1'b1;
    pvb = pval[ridx-:1];
    pcb = pval[0.5-:1];
    pvw = pval[1-:ridx];
    pcw = pval[1-:0.5];
    svb = sval[ridx-:1];
    scb = sval[0.5-:1];
    svw = sval[1-:ridx];
    scw = sval[1-:0.5];
    strvb[ridx-:1] = "a";
    strcb[0.5-:1] = "a";
    strvw[1-:ridx] = "a";
    strcw[1-:0.5] = "a";
  end
endmodule
