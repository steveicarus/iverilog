// Check that a constant function call is permitted when the call is inside
// a named block (issue #1141)

module test;

function integer f1(input integer i);

begin
  f1 = i + 1;
end

endfunction

function integer f2(input integer i);

begin : b2
  f2 = f1(i);
end

endfunction

localparam p = f2(1);

initial begin
  $display(p);
  if (p === 2)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
