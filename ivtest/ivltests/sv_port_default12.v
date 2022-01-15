// Test non-constant port default value.

module test();

integer a;
integer b;

function integer k(integer i, integer j = a+b);
  k = i + j;
endfunction

wire [31:0] x = k(1);
wire [31:0] y = k(2);

integer result;

reg fail = 0;

initial begin
  a = 1;
  b = 2;

  #0;

  result = x;
  $display(result);
  if (result !== 4) fail = 1;

  result = y;
  $display(result);
  if (result !== 5) fail = 1;

  result = k(3);
  $display(result);
  if (result !== 6) fail = 1;

  result = k(3,4);
  $display(result);
  if (result !== 7) fail = 1;

  if (fail)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
