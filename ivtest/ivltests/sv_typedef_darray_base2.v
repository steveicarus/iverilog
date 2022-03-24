// Check that a complex base type (like a packed array) of a dynamic array type
// is evaluated in the scope where the array type is declared.

localparam A = 8;

// Use type identifier for the base to force packed array
typedef logic l;
typedef l [A-1:0] T[];

module test;
  localparam A = 4;

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
