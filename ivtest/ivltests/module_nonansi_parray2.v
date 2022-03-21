// Check that it is possible to declare the data type for a packed array module
// port before the direction for non-ANSI style port declarations.

typedef logic [3:0] T1;
typedef T1 [7:0] T2;

module test(x);
  T2 x;
  output x;

  initial begin
    if ($bits(x) == $bits(T2)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
