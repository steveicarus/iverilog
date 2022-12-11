// Check that an error is reported if the number of loop indices exceeds the
// number of array dimensions in a foreach loop.

module test;

  logic a[10];

  initial begin
    foreach(a[i,j]) begin
      $display("FAILED");
    end
  end

endmodule
