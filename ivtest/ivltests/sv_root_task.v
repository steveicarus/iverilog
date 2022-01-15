task a_task (input int id);
   $display("This is task %0d.", id);
endtask

// This should print the following:
//    This is task 2.
//    This is task 1.
module top;
   initial begin
      a_task(2);
      a_task(1);
   end
endmodule
