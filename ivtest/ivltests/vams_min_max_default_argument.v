// Check min() and max() expressions as default function arguments.

module test;

  real left = 1.0;
  real right = 2.0;

  function real minimum(real value = min(left, right));
    return value;
  endfunction

  function real maximum(real value = max(left, right));
    return value;
  endfunction

  initial begin
    if (minimum() == 1.0 && maximum() == 2.0)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
