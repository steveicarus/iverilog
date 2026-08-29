module top;
  localparam integer LPX = 32'b01x;
  localparam integer LPZ = 32'b01z;
  // It is illegal to use an x or z in the dimensions.
  reg [LPX] rg1;
  reg [LPZ] rg2;

  reg arg1 [LPX];
  reg arg2 [LPZ];

  initial $display("Compiling should fail.");
endmodule
