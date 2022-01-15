module static test();

task accumulate1(input integer value, output integer result);
begin:blk
  static int acc = 1;
  acc = acc + value;
  result = acc;
end
endtask

task automatic accumulate2(input integer value, output integer result);
begin:blk
  int acc = 1;
  acc = acc + value;
  result = acc;
end
endtask

integer value;

initial begin
  static reg failed = 0;

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
