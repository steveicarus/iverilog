`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif
module top;
   reg [9:0] a;
   reg b;

   initial begin
      a = 10'h3ff;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
      b = a[15];
`else
      b = 1'bx;
`endif
      $display("A = %h, b = %b", a, b);
   end // initial begin
endmodule // top
