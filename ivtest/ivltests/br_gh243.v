module test();

bit [3:0] array[15:0];

reg failed = 0;

integer i;

initial begin
  for (i = 0; i < 16; i++) begin
    array[i] = i;
  end
  for (i = 0; i < 16; i++) begin
    $display("%b", array[i]);
    if (array[i] != i) failed = 1;
  end
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
