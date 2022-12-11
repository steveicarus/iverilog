// Check that foreach loops without an index list work as expected. This is not
// particularly useful, but it is legal code.

module test;

  logic a[10];
  int i = 0;

  initial begin
    foreach(a[]) begin
      i++;
    end

    if (i == 0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
