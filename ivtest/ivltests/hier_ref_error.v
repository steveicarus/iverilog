module hier_ref_error();

task my_task;

begin:block
end

endtask

initial my_task.missing = 0;

endmodule
