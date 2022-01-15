module test;

task automatic foo(input int id);
  #1000 $display("task %0d finished at time %0t", id, $time);
endtask

initial begin
  $display("main thread started at time %0t", $time);
  fork
    #1 foo(1);
    #2 foo(2);
  join_none
  #5;
  $display("main thread continued at time %0t", $time);
  fork
    #1 foo(3);
    #2 foo(4);
  join_any
  $display("main thread finished at time %0t", $time);
end

endmodule
