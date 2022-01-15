module pr2533175();

task fill_array;

begin:block
  reg [7:0] array[3:0];
  integer i;

  for (i = 0; i < 4; i = i + 1) begin
    array[i] = i;
  end
  for (i = 0; i < 4; i = i + 1) begin
    if (array[i] != i) begin
      $display("FAILED: %0d != %0d", array[i], i);
      $finish;
    end
  end
  $display("PASSED");
end

endtask

initial fill_array;

endmodule
