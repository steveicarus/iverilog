// Check that it is possible to declare an enum type with a scalar vector type
// as the base type.

module test;

  enum reg {
    A
  } e1;

  enum logic {
    B
  } e2;

  enum bit {
    C
  } e3;

  initial begin
    if ($bits(e1) == 1 && $bits(e2) == 1 && $bits(e3) == 1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
