// Copyright 2007, Martin Whitaker.
// This file may be freely copied for any purpose.
module memory_monitor();

reg [7:0] Memory[0:15];

reg [3:0] Index;

wire Flag1;
wire FlagI;

assign Flag1 = (Memory[1] == 0);

assign FlagI = (Memory[Index] == 0);

initial begin
  Index = 1;
  Memory[Index] = 0;
  #1 $display("Flag1 = %b, FlagI = %b", Flag1, FlagI);
  Memory[Index] = 1;
  #1 $display("Flag1 = %b, FlagI = %b", Flag1, FlagI);
end

endmodule
