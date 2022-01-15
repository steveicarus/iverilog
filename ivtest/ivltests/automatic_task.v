module automatic_task();

task automatic fill_array;

input [7:0] value;

reg   [7:0] array[3:0];

event       step;

fork
  begin
    #10 array[0] =    value; ->step;
    #10 array[1] = array[0]; ->step;
    #10 array[2] = array[1]; ->step;
    #10 array[3] = array[2]; ->step;
  end
  begin
    @step $display(array[0], array[1], array[2], array[3]);
    @step $display(array[0], array[1], array[2], array[3]);
    @step $display(array[0], array[1], array[2], array[3]);
    @step $display(array[0], array[1], array[2], array[3]);
  end
join

endtask

initial #1 fill_array(1);
initial #2 fill_array(2);

endmodule
