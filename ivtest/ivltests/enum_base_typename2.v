// Check that it is possible to declare an enum type with a type identifier plus
// packed dimensions as the the base type.

module test;

  typedef bit T;

  enum T [31:0] {
    A
  } E;

  initial begin
    if ($bits(E) == 32) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
