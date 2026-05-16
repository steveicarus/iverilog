// Check that a union restricted type parameter rejects non-union overrides.

typedef struct packed {
  logic [3:0] value;
} struct_t;

typedef union packed {
  logic [3:0] value;
  logic [3:0] other;
} union_t;

module M #(
  parameter type union T = union_t
);
endmodule

module test;

  M #(
    .T(struct_t)
  ) i_m();

  initial begin
    $display("FAILED");
  end

endmodule
