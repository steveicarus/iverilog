// Check that it is possible to declare the data type for a packed array module
// port separately from the direction for non-ANSI style port declarations.
// declarations.

typedef logic [3:0] T1;
typedef T1 [7:0] T2;

module test(x);
  output x;
  T2 x;

  initial begin
    if ($bits(x) == $bits(T2)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
