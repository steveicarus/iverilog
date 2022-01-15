module automatic_task();

task automatic fill_array;

input [7:0] value;

reg   [7:0] array[3:0];

integer i;

fork
  begin
    #10 array[0] =    value;
    #10 array[1] = array[0];
    #10 array[2] = array[1];
    #10 array[3] = array[2];
  end
  begin
    @(array[0]) $display(array[0], array[1], array[2], array[3]);
    @(array[1]) $display(array[0], array[1], array[2], array[3]);
    @(array[2]) $display(array[0], array[1], array[2], array[3]);
    @(array[3]) $display(array[0], array[1], array[2], array[3]);
  end
join

endtask

initial #1 fill_array(1);
initial #2 fill_array(2);

endmodule
