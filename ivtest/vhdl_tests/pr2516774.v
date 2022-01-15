// Catch problem where we assign to function params
module top();
  integer r;

  function integer fact;
    input   n;
    integer n;

    for (fact = 1; n > 0; n = n - 1) begin
      fact = fact * n;
    end
  endfunction // for

  initial begin
    r = fact(5);
    $display("fact(5) = %d", r);
    if (r == 120)
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule // top
