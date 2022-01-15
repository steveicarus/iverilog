
module addN
  #(parameter WID = 4)
   (input wire [WID-1:0] A,
    input wire [WID-1:0] B,
    output reg [WID:0]  Q
    /* */);

   always @* Q = A + B;

endmodule // add
