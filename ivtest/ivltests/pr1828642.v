// Copyright 2007, Martin Whitaker.
// This code may be freely copied for any purpose.
module generate_memory();

generate
  genvar	b;

  for (b = 0; b < 4; b = b + 1) begin: Byte
    reg [7:0] Data[0:3];
  end
endgenerate

integer		i;

initial begin
  for (i = 0; i < 4; i = i + 1) begin
    Byte[0].Data[i] = i*16 + 1;
    Byte[1].Data[i] = i*16 + 2;
    Byte[2].Data[i] = i*16 + 3;
    Byte[3].Data[i] = i*16 + 4;
  end
  for (i = 0; i < 4; i = i + 1) begin
    $display("%h", Byte[0].Data[i]);
    $display("%h", Byte[1].Data[i]);
    $display("%h", Byte[2].Data[i]);
    $display("%h", Byte[3].Data[i]);
  end
end

endmodule
