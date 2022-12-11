// Check that a enum can't be its own base type

module test;

  typedef T;
  typedef enum T {
    A, B
  } T;

  initial begin
    $display("FAILED");
  end

endmodule
