module automatic_error();

task automatic auto_task;

reg local;

local = 1;

endtask

initial auto_task.local = 0;

endmodule
