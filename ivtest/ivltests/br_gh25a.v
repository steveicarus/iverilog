// Regression test for GitHub issue 25
// This should result in a compile-time error when the language generation
// is 1364-2005 or earlier.

function test(input i);

begin
  test = i;
end

endfunction

module tb;

initial begin
  if (test(1))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
