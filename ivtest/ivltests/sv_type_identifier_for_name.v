// Check that for loop variables can shadow visible type identifiers.

typedef int T;

module test;

  reg failed;
  int unsigned values [3];

  initial begin
    int unsigned total;

    failed = 1'b0;
    total = 0;

    for (int unsigned T = 0; T < 3; T += 1) begin
      total += T;
      values[T] = 10 + T;
    end

    if (total != 3 || values[0] != 10 ||
        values[1] != 11 || values[2] != 12) begin
      $display("FAILED(%0d). for loop variable did not hide typedef", `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
