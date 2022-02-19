// Check that overrideing a type parameter with an expression that is not a type
// generates an error.

module M #(
  type T = int
);

  T x;

  initial begin
    $display("FAILED");
  end

endmodule

module test;

  M #(
    .T(10)
  ) m();

endmodule

