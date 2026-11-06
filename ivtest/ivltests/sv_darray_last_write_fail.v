// Check that the last element index is rejected for dynamic array writes.

module test;
  int a[];

  initial begin
    a = new[2];
    a[$] = 9; // Error: The $ index is only allowed for queues.
  end
endmodule
