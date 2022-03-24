// Check that a complex base type (like a struct) of an array type is evaluated
// in the scope where the array type is declared.

localparam A = 8;

typedef struct packed {
  logic [A-1:0] x;
} T[1:0];

module test;
  localparam A = 4;

  T x;

  initial begin
    x[0] = 8'hff;
    if (x[0] === 8'hff) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
