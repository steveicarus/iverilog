`begin_keywords "1364-2005"
module automatic_error();

reg  global;

task automatic auto_task;

begin:block
  reg local;

  global <= @(local) 0;
end

endtask

endmodule
`end_keywords
