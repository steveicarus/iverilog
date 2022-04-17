// Check that circular type definitions are detected and an error is reported.

module test;
  typedef T1;
  typedef T1 T2;
  typedef T2 T1;

  T2 x;

  initial begin
    $display("FAILED");
  end

endmodule
