// Check that it is possible to declare the data type for a struct type module
// port separately from the direction for non-ANSI style port declarations.
// declarations.

typedef struct packed {
  reg [31:0] x;
  reg [7:0] y;
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
