// Check that non-blocking event control assignment to a part select on a vector
// array works when using a variable index.

module top;
  reg pass = 1'b1;

  integer i = 0;
  integer j = 1;
  integer count;
  reg [2:0] icount;
  reg clk = 0;
  reg [3:0] in = 4'h0;
  reg [7:0] result [1:0];

  always #10 clk = ~clk;
  always #20 in = in + 4'h1;

  initial begin
    count = 3;
    i = 0;
    result[j][i+:4] <= repeat(count) @(posedge clk) in;
    if ($simtime != 0 || result[j] !== 8'bx) begin
      $display("Failed repeat(3) blocked at %0t, expected 8'hxx, got %h",
               $simtime, result[j]);
      pass = 1'b0;
    end
    @(result[j]);
    if ($simtime != 50 || result[j] !== 8'hx0) begin
      $display("Failed repeat(3) at %0t, expected 8'hx0, got %h",
               $simtime, result[j]);
      pass = 1'b0;
    end

    #15;
    count = 0;
    result[j][i+4+:4] <= repeat(count) @(posedge clk) in;
    @(result[j]); // Reals happen faster so they can use an #0, vectors are slower.
    if ($simtime != 65 || result[j] !== 8'h30) begin
      $display("Failed repeat(0) at %0t, expected 8'h30, got %h",
               $simtime, result[j]);
      pass = 1'b0;
    end

    #20;
    count = -1;
    result[j][i+5+:4] <= repeat(count) @(posedge clk) in;
    @(result[j]); // Reals happen faster so they can use an #0, vectors are slower.
    if ($simtime != 85 || result[j] !== 8'h90) begin
      $display("Failed repeat(-1) at %0t, expected 8'h80, got %h",
               $simtime, result[j]);
      pass = 1'b0;
    end

    #20;
    result[j][i+4+:4] <= @(posedge clk) 4'h0;
    result[j][i+4+:4] <= @(posedge clk) in; // This one sets the final value.
    @(result[j]);
    if ($simtime != 110 || result[j] !== 8'h50) begin
      $display("Failed @ at %0t, expected 8'h50, got %h",
               $simtime, result[j]);
      pass = 1'b0;
    end

    icount = 3'd2;
    result[j][i+:4] <= @(posedge clk) 4'h1;
    result[j][i+4+:4] <= repeat(icount) @(posedge clk) 4'h2;
    result[j][i+1-:4] <= repeat(3) @(posedge clk) 4'h3;
    @(result[j]);
    if ($simtime != 130 || result[j] !== 8'h51) begin
      $display("Failed first @ at %0t, expected 8'h51, got %h",
               $simtime, result[j]);
      pass = 1'b0;
    end
    @(result[j]);
    if ($simtime != 150 || result[j] !== 8'h21) begin
      $display("Failed second @ at %0t, expected 8'h21, got %h",
               $simtime, result[j]);
      pass = 1'b0;
    end
    @(result[j]);
    if ($simtime != 170 || result[j] !== 8'h20) begin
      $display("Failed third @ at %0t, expected 8'h20, got %h",
               $simtime, result[j]);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
    $finish;
  end
endmodule
