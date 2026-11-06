// Check that the last element index is rejected for dynamic array reads.

module test;
  int a[];

  initial begin
    a = new[2];
    $display("%0d", a[$]); // Error: The $ index is only allowed for queues.
  end
endmodule
