module top;
  reg pass = 1'b1;

  integer count;
  reg clk = 0, in = 0;
  reg result;

  always #10 clk = ~clk;
  always #20 in = ~in;

  initial begin
  count = 3;
  result = repeat(count) @(posedge clk) in;
  if ($simtime != 30 && result != 1'b0) begin
    $display("Failed blocking repeat(3) at %0t, expected 1'b0, got %b",
             $simtime, result);
    pass = 1'b0;
  end

  #15;
  count = 0;
  result = repeat(count) @(posedge clk) in;
  if ($simtime != 45 && result != 1'b1) begin
    $display("Failed blocking repeat(0) at %0t, expected 1'b1, got %b",
             $simtime, result);
    pass = 1'b0;
  end

  #20;
  count = -1;
  result = repeat(count) @(posedge clk) in;
  if ($simtime != 55 && result != 1'b0) begin
    $display("Failed blocking repeat(0) at %0t, expected 1'b0, got %b",
             $simtime, result);
    pass = 1'b0;
  end

  if (pass) $display("PASSED");
  $finish;
  end
endmodule
