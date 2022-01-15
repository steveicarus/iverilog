module automatic_error();

reg  global;

task automatic auto_task;

reg local;

begin:block
  force global = local;
end

endtask

endmodule
