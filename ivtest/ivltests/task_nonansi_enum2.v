// Check that it is possible to declare the data type for an enum type task port
// before the direction for non-ANSI style port declarations.

module test;

  typedef enum integer {
    A, B
  } T;

  task t;
    T x;
    input x;
    if (x == B && $bits(x) == $bits(T)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial t(B);

endmodule
