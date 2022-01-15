
/*
 * Sign extend input
 * T can be 0 for <<, 1 for >>, 2 for <<< or 3 for >>>.
 */
module sign_ext
  #(parameter WI = 4, WO = 6)
   (input wire signed [WI-1:0] D,
    output wire signed [WO-1:0] Q
    /* */);

   assign Q = D;

endmodule

