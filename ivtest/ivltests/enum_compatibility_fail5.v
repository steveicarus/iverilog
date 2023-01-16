// Check that an error is reported for implicit cast to enum for task arguments.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  task t(E e);
    $display("FAILED");
  endtask

  initial begin
    t(10); // This should fail. Implicit cast to enum.
  end

endmodule
