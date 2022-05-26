// Check that assignment operators on real arrays are supported.

module test;

  real r[1:0];
  integer i = 1;

  initial begin
    // Immediate index
    r[0] = 8.0;
    r[0] += 1.0;
    r[0] -= 2.0;
    r[0] *= 3.0;
    r[0] /= 7.0;
    r[0]++;
    r[0]--;

    // Variable index
    r[i] = 8.0;
    r[i] += 1.0;
    r[i] -= 2.0;
    r[i] *= 3.0;
    r[i] /= 7.0;
    r[i]++;
    r[i]--;

    if (r[0] == 3.0 && r[1] == 3.0) begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected %f, got %f and %f", 3.0, r[0], r[1]);
    end
  end

endmodule
