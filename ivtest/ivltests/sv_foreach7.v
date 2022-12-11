// Check that foreach loops with only empty indices works as expected. This is
// not particularly useful, but it is legal code.

module test;

  logic a[2][3][4];
  int i = 0;

  initial begin
    foreach(a[,,]) begin
      i++;
    end

    if (i == 0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
