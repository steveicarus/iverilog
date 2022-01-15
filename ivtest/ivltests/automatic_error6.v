module automatic_error();

reg  global;

task automatic auto_task;

reg local;

begin:block
  deassign local;
end

endtask

endmodule
