
/*
 * Generate a barrel shifter of arbitrary width.
 * T can be 0 for <<, 1 for >>, 2 for <<< or 3 for >>>.
 */
module shift
  #(parameter WI = 4, WS = 4, parameter WO = 6)
   (input wire [WI-1:0] D,
    input wire [WS-1:0] S,
    output wire [WO-1:0] SHL,
    output wire [WO-1:0] SHR,
    output wire signed [WO-1:0] ASHL,
    output wire signed [WO-1:0] ASHR
    /* */);

   wire signed [WI-1:0] DS;
   assign DS = D;

   assign SHL = D << S ;
   assign SHR = D >> S ;
   assign ASHL = DS <<< S ;
   assign ASHR = DS >>> S ;

endmodule

