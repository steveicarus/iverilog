module top;
  integer res;
  initial begin
    $hello;
    $check_sys_task;
    res = $check_sys_func;
    $hello;
  end
endmodule
