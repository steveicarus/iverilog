// Copyright 2008, Martin Whitaker.
// This file may be freely copied for any purpose.

module array_port_events();

reg  [1:0]	Data[3:0];

reg  [1:0]	Index;

reg  [1:0]	Value;

integer		i;
integer		j;

initial begin
  for (i = 0; i < 4; i = i + 1) begin
    Index = i;
    for (j = 0; j < 4; j = j + 1) begin
      Data[i] = i + j;
      #2;
    end
  end
end

always @* begin
  case (Index)
    0 : Value = Data[0];
    1 : Value = Data[1];
    2 : Value = Data[2];
    3 : Value = Data[3];
  endcase
end

always @(Value) begin
  #1 $display("%d", Value);
end

endmodule
