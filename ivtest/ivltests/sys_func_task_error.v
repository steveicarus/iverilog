module top;
  reg [79:0] str;
  integer val;

  initial begin
    // This should be a not defined in any module message.
    $this_icarus_call_should_not_exist;
    str = "5";
    // This should be a system function is being called as a task error.
    $sscanf(str, "%d", val);
    // This should be a system task is being called as a function error.
    val = $display;
  end
endmodule
