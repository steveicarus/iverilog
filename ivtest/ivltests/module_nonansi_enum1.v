// Check that it is possible to declare the data type for a enum type module
// port separately from the direction for non-ANSI style port declarations.
// declarations.

typedef enum integer {
  A, B
} T;

module test(x);
  output x;
  T x;

  initial begin
    if ($bits(x) == $bits(T)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
