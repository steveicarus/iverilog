module automatic test();

task static accumulate1(input integer value, output integer result);
  static int acc = 1;
  acc = acc + value;
  result = acc;
endtask

task accumulate2(input integer value, output integer result);
  int acc = 1;
  acc = acc + value;
  result = acc;
endtask

integer value;

reg failed = 0;

initial begin
  accumulate1(2, value);
  $display("%d", value);
  if (value !== 3) failed = 1;

  accumulate1(3, value);
  $display("%d", value);
  if (value !== 6) failed = 1;

  accumulate2(2, value);
  $display("%d", value);
  if (value !== 3) failed = 1;

  accumulate2(3, value);
  $display("%d", value);
  if (value !== 4) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
