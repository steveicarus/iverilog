// Check that a class restricted type parameter rejects non-class defaults.

module test #(
  parameter type class T = int
);

  initial begin
    $display("FAILED");
  end

endmodule
