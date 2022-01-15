// Copyright 2008, Martin Whitaker.
// This file may be freely copied for any purpose.

module array_word_part_select();
reg pass = 1'b1;

reg  [3:0]	Data[15:0];

reg  [3:0]	Index;

wire [2:0]	Value;


assign Value = Data[Index][2:0];

integer		i;

initial begin
  for (i = 0; i < 16; i = i + 1) begin
    Data[i] = i;
  end
  for (i = 0; i < 16; i = i + 1) begin
    #2 Index = i;
  end
  #2 if (pass) $display("PASSED");
end

always @(Index) begin
//  $display(Index,, Value);
  #1 if (Value !== Index % 8) begin
    $display("Failed: testing index %d, expected %d, got %d", Index,
             Index % 8, Value);
    pass = 1'b0;
  end
end

endmodule
