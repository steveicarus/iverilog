// Copyright 2008, Martin Whitaker.
// This file may be freely copied for any purpose.

module scan_int_array();

integer f;

integer i;
integer n;

integer v[0:3];

initial begin
  f = $fopen("work/temp.txt", "w");
  for (i = 0; i < 4; i = i + 1) begin
    $fdisplay(f, "%d", i);
  end
  $fclose(f);

  f = $fopen("work/temp.txt", "r");
  for (i = 0; i < 4; i = i + 1) begin
    n = $fscanf(f, " %d ", v[i]);
  end
  $fclose(f);

  for (i = 0; i < 4; i = i + 1) begin
    $display("%1d", v[i]);
  end
end

endmodule
