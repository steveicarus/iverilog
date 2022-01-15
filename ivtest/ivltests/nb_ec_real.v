module top;
  reg pass = 1'b1;

  integer count;
  reg [2:0] icount;
  reg clk = 0;
  real in = 0.0;
  real result = -1.0;

  always #10 clk = ~clk;
  always #20 in = in + 1.0;

  initial begin
    count = 3;
    result <= repeat(count) @(posedge clk) in;
    if ($simtime != 0 || result != -1.0) begin
      $display("Failed repeat(3) blocked at %0t, expected -1.0, got %f",
               $simtime, result);
      pass = 1'b0;
    end
    @(result);
    if ($simtime != 50 || result != 0.0) begin
      $display("Failed repeat(3) at %0t, expected 0.0, got %f",
               $simtime, result);
      pass = 1'b0;
    end

    #15;
    count = 0;
    result <= repeat(count) @(posedge clk) in;
    #0; // This may not work since there is no delay.
    if ($simtime != 65 || result != 3.0) begin
      $display("Failed repeat(0) at %0t, expected 3.0, got %f",
               $simtime, result);
      pass = 1'b0;
    end

    #20;
    count = -1;
    result <= repeat(count) @(posedge clk) in;
    #0; // This may not work since there is no delay.
    if ($simtime != 85 || result != 4.0) begin
      $display("Failed repeat(-1) at %0t, expected 4.0, got %f",
               $simtime, result);
      pass = 1'b0;
    end

    #20;
    result <= @(posedge clk) 0.0;
    result <= @(posedge clk) in; // This one sets the final value.
    @(result);
    if ($simtime != 110 || result != 5.0) begin
      $display("Failed @ at %0t, expected 5.0, got %f",
               $simtime, result);
      pass = 1'b0;
    end

    icount = 3'd2;
    result <= @(posedge clk) 1.0;
    result <= repeat(icount) @(posedge clk) 2.0;
    result <= repeat(3) @(posedge clk) 3.0;
    @(result);
    if ($simtime != 130 || result != 1.0) begin
      $display("Failed first @ at %0t, expected 1.0, got %f",
               $simtime, result);
      pass = 1'b0;
    end
    @(result);
    if ($simtime != 150 || result != 2.0) begin
      $display("Failed second @ at %0t, expected 2.0, got %f",
               $simtime, result);
      pass = 1'b0;
    end
    @(result);
    if ($simtime != 170 || result != 3.0) begin
      $display("Failed third @ at %0t, expected 3.0, got %f",
               $simtime, result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
    $finish;
  end
endmodule
