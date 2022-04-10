// Check that it is possible to declare the data type for an atom2 type module
// port before the direction for non-ANSI style port declarations.

module test(x, y, z, w);
  byte x;
  shortint y;
  int z;
  longint w;
  output x;
  output y;
  output z;
  output w;

  initial begin
    if ($bits(x) == 8 && $bits(y) == 16 &&
        $bits(z) == 32 && $bits(w) == 64) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
