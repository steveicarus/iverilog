module top;
  parameter in = "First   Second    Third    15";
  reg pass;
  integer res, arg4;
  reg  [32*8:1] arg1, arg2, arg3;

  initial begin
    pass = 1'b1;

    res = $sscanf(in, "%s%s%s%d", arg1, arg2, arg3, arg4);

    if (res != 4) begin
      $display("FAILED: wrong number of arguments, expected 4, got %0d", res);
      pass = 1'b0;
    end

    if (arg1[5*8:1] !== "First") begin
      $display("FAILED: arg1, expected \"First\", got \"%0s\"", arg1);
      pass = 1'b0;
    end

    if (arg2[6*8:1] !== "Second") begin
      $display("FAILED: arg2, expected \"Second\", got \"%0s\"", arg2);
      pass = 1'b0;
    end

    if (arg3[5*8:1] !== "Third") begin
      $display("FAILED: arg3, expected \"Third\", got \"%0s\"", arg3);
      pass = 1'b0;
    end

    if (arg4 != 15) begin
      $display("FAILED: arg4, expected 15, got %0d", arg4);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");

  end
endmodule
