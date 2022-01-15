module top;
  reg pass;
  integer in1, in2, res;

  initial begin
    pass = 1'b1;

    in1 = 1; in2 = 2;
    res = in1 ** in2;
    if (res != 1) begin
      $display("Failed: 1 ** 2, expected 1, got %0d", res);
      pass = 1'b0;
    end

    in1 = 2; in2 = 3;
    res = in1 ** in2;
    if (res != 8) begin
      $display("Failed: 2 ** 3, expected 8, got %0d", res);
      pass = 1'b0;
    end

    in1 = -2; in2 = 2;
    res = in1 ** in2;
    if (res != 4) begin
      $display("Failed: -2 ** 2, expected 4, got %0d", res);
      pass = 1'b0;
    end

    in1 = -2; in2 = 3;
    res = in1 ** in2;
    if (res != -8) begin
      $display("Failed: -2 ** 3, expected -8, got %0d", res);
      pass = 1'b0;
    end

    in1 = 1; in2 = -1;
    res = in1 ** in2;
    if (res != 1) begin
      $display("Failed: 1 ** -1, expected 1, got %0d", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
