/*
 * Based on PR#1008
 */

`timescale  1 ps / 1 ps

module star;

reg a;
reg b;

initial begin
   $monitor("b = %b", b);
    #1;
    a = 1;
    #2;
    a = 0;
    #2;
    a = 1;
end

/* This generated the error:
   :0: internal error: NetProc::nex_output not implemented
   Before CVS 20040630 */
always @* begin
    b = #1 ~a;
end

endmodule // star
