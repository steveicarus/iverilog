module automatic_error();

reg  global;

task automatic auto_task;

reg local;

begin:block
  assign global = local;
end

endtask

endmodule
