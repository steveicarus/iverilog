// Check that it is not possible to assign a dynamic array with different
// element type width.

module test;

  int d1[];
  shortint d2[];

  initial begin
    d1 = d2;
    $dispaly("FAILED");
  end

endmodule
