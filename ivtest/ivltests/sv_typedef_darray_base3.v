// Check that a vector base type of a dynamic array type gets evaluated in the
// right scope if the base type is defined in a different scope than the array
// type.

localparam A = 8;

typedef logic [A-1:0] Base;

module test;
  localparam A = 4;

  typedef Base T[];

  T x;

  initial begin
    x = new [1];
    x[0] = 8'hff;
    if (x[0] === 8'hff) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
