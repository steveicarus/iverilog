module top;
  real rval;

  always @(rval) $display("any change");
  always @(posedge rval) $display("posedge");
  always @(negedge rval) $display("negedge");
  always @(edge rval) $display("edge");

  initial begin
    #1 rval = 1.0;
    #1 rval = 0.0;
  end
endmodule
