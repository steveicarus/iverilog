module br932b();

string  filename;

integer f;

integer i;
integer n;

integer v[0:3];

reg     failed;

initial begin
  f = $fopen("work/temp.txt", "w");
  for (i = 0; i < 4; i = i + 1) begin
    $fdisplay(f, "%d", i);
  end
  $fclose(f);

  filename = "work/temp.txt";
  f = $fopen(filename, "r");
  for (i = 0; i < 4; i = i + 1) begin
    n = $fscanf(f, " %d ", v[i]);
  end
  $fclose(f);

  failed = 0;
  for (i = 0; i < 4; i = i + 1) begin
    $display("%1d", v[i]);
    if (v[i] !== i) failed = 1;
  end
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
