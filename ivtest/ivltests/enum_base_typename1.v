// Check that it is possible to declare an enum type with a type identifier that
// resolves to an integer type as the base type.

module test;

  typedef integer T;

  enum T {
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
