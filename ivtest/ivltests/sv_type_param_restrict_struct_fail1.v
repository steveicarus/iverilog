// Check that a struct restricted type parameter rejects non-struct defaults.

typedef union packed {
  logic [3:0] value;
  logic [3:0] other;
} union_t;

module test #(
  parameter type struct T = union_t
);

  initial begin
    $display("FAILED");
  end

endmodule
