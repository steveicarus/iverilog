// Check that trying to override a type parameter using a defparam statement
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

  M m();
  defparam m.T = real; // Error, this is illegal

endmodule
