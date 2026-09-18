// Check shadowing between type and value declarations.

typedef logic [15:0] type_name;
integer value_name;

module test;

  integer type_name;
  typedef logic [7:0] value_name;
  reg failed;

  `define check(value, expected) \
    if ((value) !== (expected)) begin \
      $display("FAILED(%0d). Expected %0d, got %0d", `__LINE__, \
               expected, value); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    // A nearer value declaration hides an outer type declaration.
    `check($bits(type_name), 32);

    // A nearer type declaration hides an outer value declaration.
    `check($bits(value_name), 8);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
