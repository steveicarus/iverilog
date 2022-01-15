module automatic_error();

task automatic auto_task;

reg local;

begin:block
  local <= #1 0;
end

endtask

endmodule
