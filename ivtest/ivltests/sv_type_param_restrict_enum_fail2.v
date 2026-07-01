// Check that an enum restricted type parameter rejects non-enum overrides.

typedef enum bit {
  ENUM_VALUE
} enum_t;

module M #(
  parameter type enum T = enum_t
);
endmodule

module test;

  M #(
    .T(int)
  ) i_m();

  initial begin
    $display("FAILED");
  end

endmodule
