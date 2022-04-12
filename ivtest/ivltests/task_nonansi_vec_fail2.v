// Check that it is an error to declare a non-ANSI task port without implicit
// packed dimensions if it is later redeclared as a vector typed variable and
// the vector type is not a scalar.

module test;

  task t;
    input [7:0] x;
    reg x;
    $display("FAILED");
  endtask

  initial begin
    t(10);
  end

endmodule
