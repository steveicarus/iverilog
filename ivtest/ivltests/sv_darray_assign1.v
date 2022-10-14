// Check that dynamic arrays with compatible packed base types can be assigned
// to each other. Even if the element types are not identical.

module test;

  typedef bit [31:0] T1;
  typedef bit [31:0] T2[];

  // For two packed types to be compatible they need to have the same packed
  // width, both be 2-state or 4-state and both be either signed or unsigned.
  bit [32:1] d1[];
  bit [7:0][3:0] d2[];
  int unsigned d3[];
  T1 d4[];
  T2 d5;

  initial begin
    d1 = new[1];
    d2 = d1;
    d3 = d2;
    d4 = d3;
    d5 = d4;
    d1 = d5;

    $display("PASSED");
  end

endmodule
