module automatic_error();

reg  global;

task automatic auto_task;

reg local;

begin:block
  release local;
end

endtask

endmodule
