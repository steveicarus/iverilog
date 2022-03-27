// Check that it is possible to declare an enum type without an explicit base
// type. In this case the base type should default to `int`.

module test;

  enum {
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
