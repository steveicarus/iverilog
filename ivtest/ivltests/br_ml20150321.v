// Regression test for bug reported by Niels Moeller on 21-Mar-2015 via
// iverilog-devel mailing list. Extended to cover similar problems. This
// is just testing compiler error recovery.

module test();

integer array[3:0];

integer i1;

always @* begin
  for (i1 = 0; i1 < 4; i1 = i1 + 1) begin
    array[i1] = undeclared;
  end
end

integer i2;

always @* begin
  for (i2 = 0; i2 < 4; i2 = i2 + 1) begin
    undeclared = array[i2];
  end
end

integer i3;

always @* begin
  for (i3 = undeclared; i3 < 4; i3 = i3 + 1) begin
    array[i3] = i3;
  end
end

integer i4;

always @* begin
  for (i4 = 0; i4 < undeclared; i4 = i4 + 1) begin
    array[i4] = i4;
  end
end

integer i5;

always @* begin
  for (i5 = 0; i5 < 4; i5 = i5 + undeclared) begin
    array[i5] = i5;
  end
end

integer i6;

always @* begin
  i6 = 0;
  while (i6 < undeclared) begin
    array[i6] = i6;
    i6 = i6 + 1;
  end
end

integer i7;

always @* begin
  i7 = 0;
  while (i7 < 4) begin
    array[i7] = undeclared;
    i7 = i7 + 1;
  end
end

integer i8;

always @* begin
  i8 = 0;
  repeat (undeclared) begin
    array[i8] = i8;
    i8 = i8 + 1;
  end
end

integer i9;

always @* begin
  i9 = 0;
  repeat (4) begin
    array[i9] = undeclared;
    i9 = i9 + 1;
  end
end

endmodule
