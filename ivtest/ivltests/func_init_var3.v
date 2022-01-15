module automatic test();

function static integer accumulate1(input integer value);
  static int acc = 1;
  acc = acc + value;
  return acc;
endfunction

function integer accumulate2(input integer value);
  int acc = 1;
  acc = acc + value;
  return acc;
endfunction

localparam value1 = accumulate1(2);
localparam value2 = accumulate1(3);
localparam value3 = accumulate2(2);
localparam value4 = accumulate2(3);

integer value;

reg failed = 0;

initial begin
  $display("%d", value1);
  if (value1 !== 3) failed = 1;

  $display("%d", value2);
  if (value2 !== 4) failed = 1;

  $display("%d", value3);
  if (value3 !== 3) failed = 1;

  $display("%d", value4);
  if (value4 !== 4) failed = 1;

  value = accumulate1(2);
  $display("%d", value);
  if (value !== 3) failed = 1;

  value = accumulate1(3);
  $display("%d", value);
  if (value !== 6) failed = 1;

  value = accumulate2(2);
  $display("%d", value);
  if (value !== 3) failed = 1;

  value = accumulate2(3);
  $display("%d", value);
  if (value !== 4) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
