module top;
  localparam integer LPX = 32'b01x;
  localparam integer LPZ = 32'b01z;
  // It is illegal to use an x or z in the dimensions.
  reg [LPX:0] rg1;
  reg [LPZ:0] rg2;
  reg [0:LPX] rg3;
  reg [0:LPZ] rg4;

  reg arg1 [LPX:0];
  reg arg2 [LPZ:0];
  reg arg3 [0:LPX];
  reg arg4 [0:LPZ];

  initial $display("Compiling should fail.");
endmodule
