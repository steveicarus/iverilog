// Check that it is possible to omit a dimensions in a foreach loop by not
// specifying a loop identifiers for the dimension.

module test;

  logic a[2][3][4];
  int k = 0;

  initial begin
    foreach(a[i,,j]) begin
      $display(i, j);
      k++;
    end

    if (k == 8) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
