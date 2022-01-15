module constfunc2();

function integer factorial;

input integer n;

begin
  if (n > 1)
    factorial = n * factorial(n - 1);
  else
    factorial = n;
end

endfunction

localparam value1 = factorial(1);
localparam value2 = factorial(2);
localparam value3 = factorial(3);
localparam value4 = factorial(4);
localparam value5 = factorial(5);
localparam value6 = factorial(6);

initial begin
  $display("value 1 = %0d", value1);
  $display("value 2 = %0d", value2);
  $display("value 3 = %0d", value3);
  $display("value 4 = %0d", value4);
  $display("value 5 = %0d", value5);
  $display("value 6 = %0d", value6);
  if ((value1 === 1)
  &&  (value2 === 2)
  &&  (value3 === 6)
  &&  (value4 === 24)
  &&  (value5 === 120)
  &&  (value6 === 720))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
