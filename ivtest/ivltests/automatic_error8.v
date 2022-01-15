module automatic_error();

reg  global;

task automatic auto_task;

reg local;

begin:block
  force local = global;
end

endtask

endmodule
