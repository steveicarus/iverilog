// Check that it is possible to declare an enum type with the integer type as
// the base type.

module test;

  enum integer {
    A
  } E;

  initial begin
    if ($bits(E) == $bits(integer)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
