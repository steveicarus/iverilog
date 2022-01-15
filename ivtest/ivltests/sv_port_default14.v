// Test default value for output port
// This should work, but isn't supported yet

module test();

integer a;
integer b;
integer c;

task k(input integer i = a, output integer j = b);
  j = i;
endtask

integer result;

reg fail = 0;

initial begin
  a = 1;
  b = 2;

  k(3,c);
  $display(a,,b,,c);
  if (a !== 1 || b !== 2 || c !== 3) fail = 1;

  k(,c);
  $display(a,,b,,c);
  if (a !== 1 || b !== 2 || c !== 1) fail = 1;

  k(4,);
  $display(a,,b,,c);
  if (a !== 1 || b !== 4 || c !== 1) fail = 1;

  k();
  $display(a,,b,,c);
  if (a !== 1 || b !== 1 || c !== 1) fail = 1;

  if (fail)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
