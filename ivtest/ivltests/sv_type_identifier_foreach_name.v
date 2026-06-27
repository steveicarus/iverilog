// Check that foreach loop variables can shadow visible type identifiers.

typedef int I;
typedef int J;

module test;

  reg failed;
  int array [2][2];

  initial begin
    failed = 1'b0;

    foreach (array[I,J]) begin
      array[I][J] = I * 10 + J;
    end

    if (array[0][0] != 0 || array[0][1] != 1 ||
        array[1][0] != 10 || array[1][1] != 11) begin
      $display("FAILED(%0d). foreach indices did not hide typedefs", `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
