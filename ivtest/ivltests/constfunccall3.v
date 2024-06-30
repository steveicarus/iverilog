// Check that a constant function call is permitted when the function is
// provided by a package.

module m1;

function integer f1(input integer i);

begin
  f1 = i + 1;
end

endfunction

endmodule

module test;

function integer f2(input integer i);

begin
  f2 = m1.f1(i);
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
