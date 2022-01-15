module automatic_task();

reg [7:0] array[3:0];

task automatic fill_array;

input [7:0] value;

integer i, j;

fork
  for (i = 0; i < 4; i = i + 1) begin
    #10 array[i] = value;
  end
  for (j = 0; j < 4; j = j + 1) begin
    @(array[j]) $display(array[0], array[1], array[2], array[3]);
    @(array[j]) $display(array[0], array[1], array[2], array[3]);
  end
join

endtask

initial #1 fill_array(1);
initial #2 fill_array(2);

endmodule
