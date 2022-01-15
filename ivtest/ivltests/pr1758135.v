// Copyright 2007, Martin Whitaker.
// This code may be freely copied for any purpose.

module display_index_test();

reg  [2:0]	A[1:4];

integer		i;

initial begin
  for (i = 1; i <= 4; i = i + 1) begin
    A[i] = i;
  end
  for (i = 1; i <= 4; i = i + 1) begin
    $display("%d", A[i]);
  end
end

endmodule
