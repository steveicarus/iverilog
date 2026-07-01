// Check that a union restricted type parameter rejects non-union defaults.

typedef struct packed {
  logic [3:0] value;
} struct_t;

module test #(
  parameter type union T = struct_t
);

  initial begin
    $display("FAILED");
  end

endmodule
