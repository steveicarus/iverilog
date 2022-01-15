module test ();

  initial begin : local_vars
    integer i, sum;
    sum = 0;
    for (i = 1; i <= 10; i = i + 1)
      sum = sum + i;
    $display("sum(1..10) = %d", sum);
    if (sum == 55)
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule
