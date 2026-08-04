module top;
  localparam integer LPX = 32'b01x;
  localparam integer LPZ = 32'b01z;
  // It is illegal to use an x or z in the dimensions.
  localparam [LPX:0] pr1 = 1;
  localparam [LPZ:0] pr2 = 2;
  localparam [0:LPX] pr3 = 1;
  localparam [0:LPZ] pr4 = 2;

  initial $display("Compiling should fail.");
endmodule
