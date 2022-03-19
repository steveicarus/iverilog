// Check that it is possible to declare an enum type with an atom2 type as the
// base type.

module test;

  enum byte {
    A
  } e1;

  enum shortint {
    B
  } e2;

  enum int {
    C
  } e3;

  enum longint {
    D
  } e4;

  initial begin
    if ($bits(e1) == 8 && $bits(e2) == 16 &&
        $bits(e3) == 32 && $bits(e4) == 64) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
