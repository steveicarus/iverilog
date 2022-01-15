// This is a simple test of automatic functions
module autof();
  reg [7:0] result;

  function automatic [7:0] fact;
    input [7:0] n;
    if (n == 0)
      fact = 1;
    else
      fact = n * fact(n-1);
  endfunction // fact

  initial begin
    result = fact(4);
    if (result == 24)
      $display("PASSED");
    else
      $display("FAILED -- Expected 24 but got %d", result);
  end

endmodule // autof
