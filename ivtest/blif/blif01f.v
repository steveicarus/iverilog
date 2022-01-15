
module muxN
  #(parameter WID = 4, parameter SWID = 2)
   (input wire [WID-1:0]  D,
    input wire [SWID-1:0] S,
    output wire		  Q
    /* */);

   assign Q = D[S];

endmodule // add
