module top;
  reg pass = 1'b1;

  integer count;
  reg [2:0] icount;
  reg clk = 0;
  reg [3:0] in = 4'h0;
  reg [3:0] result [1:0];

  always #10 clk = ~clk;
  always #20 in = in + 4'h1;

  initial begin
    count = 3;
    result[0] <= repeat(count) @(posedge clk) in;
    if ($simtime != 0 || result[0] !== 4'bx) begin
      $display("Failed repeat(3) blocked at %0t, expected 4'hx, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end
    @(result[0]);
    if ($simtime != 50 || result[0] !== 4'h0) begin
      $display("Failed repeat(3) at %0t, expected 4'h0, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end

    #15;
    count = 0;
    result[0] <= repeat(count) @(posedge clk) in;
    @(result[0]); // Reals happen faster they can use an #0, vectors are slower.
    if ($simtime != 65 || result[0] !== 4'h3) begin
      $display("Failed repeat(0) at %0t, expected 4'h3, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end

    #20;
    count = -1;
    result[0] <= repeat(count) @(posedge clk) in;
    @(result[0]); // Reals happen faster they can use an #0, vectors are slower.
    if ($simtime != 85 || result[0] !== 4'h4) begin
      $display("Failed repeat(-1) at %0t, expected 4'h4, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end

    #20;
    result[0] <= @(posedge clk) 4'h0;
    result[0] <= @(posedge clk) in; // This one sets the final value.
    @(result[0]);
    if ($simtime != 110 || result[0] !== 4'h5) begin
      $display("Failed @ at %0t, expected 4'h5, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end

    icount = 3'd2;
    result[0] <= @(posedge clk) 4'h1;
    result[0] <= repeat(icount) @(posedge clk) 4'h2;
    result[0] <= repeat(3) @(posedge clk) 4'h3;
    @(result[0]);
    if ($simtime != 130 || result[0] !== 4'h1) begin
      $display("Failed first @ at %0t, expected 4'h1, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end
    @(result[0]);
    if ($simtime != 150 || result[0] !== 4'h2) begin
      $display("Failed second @ at %0t, expected 4'h2, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end
    @(result[0]);
    if ($simtime != 170 || result[0] !== 4'h3) begin
      $display("Failed third @ at %0t, expected 4'h3, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
    $finish;
  end
endmodule
