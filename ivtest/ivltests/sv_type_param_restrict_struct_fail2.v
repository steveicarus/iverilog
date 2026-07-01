// Check that a struct restricted type parameter rejects non-struct overrides.

typedef struct packed {
  logic [3:0] value;
} struct_t;

typedef union packed {
  logic [3:0] value;
  logic [3:0] other;
} union_t;

module M #(
  parameter type struct T = struct_t
);
endmodule

module test;

  M #(
    .T(union_t)
  ) i_m();

  initial begin
    $display("FAILED");
  end

endmodule
