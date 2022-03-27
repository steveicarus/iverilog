// Check that it is possible to declare an enum type with the time type as the
// base type.

module test;

  enum time {
    A
  } E;

  initial begin
    if ($bits(E) == 64) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
