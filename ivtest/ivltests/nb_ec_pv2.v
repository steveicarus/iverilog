`begin_keywords "1364-2005"
module top;
  reg pass = 1'b1;

  reg clk = 0;
  reg [7:0] result;
  reg [3:0] bit;
  integer count;

  always #10 clk = ~clk;

  initial begin
    // Since the bit is not defined this assignment will not happen.
    // We will check to verify this fact 1 time step after it should
    // happen (50).
    #0;
    result[bit] <= repeat(3) @(posedge clk) 1'b0;
    if ($simtime != 0 || result !== 8'bx) begin
      $display("Failed repeat(3) blocked at %0t, expected 8'hxx, got %h",
               $simtime, result);
      pass = 1'b0;
    end
    #51;
    if (result !== 8'hxx) begin
      $display("Failed repeat(3) at %0t, expected 8'hxx, got %h",
               $simtime, result);
      pass = 1'b0;
    end

    bit = 0;
    result[bit] <= @(posedge clk) 4'h0;
    @(result)
    if ($simtime != 70 || result !== 8'bxxxxxxx0) begin
      $display("Failed repeat(3) at %0t, expected 8'bxxxxxxx0, got %h",
               $simtime, result);
      pass = 1'b0;
    end

    // These should execute as if there was no event control
    count = 0;
    result[bit] <= repeat(count) @(posedge clk) 1'b1;
    #1
    if ($simtime != 71 || result !== 8'bxxxxxxx1) begin
      $display("Failed @ at %0t, expected 8'bxxxxxxx1, got %h",
               $simtime, result);
      pass = 1'b0;
    end

    count = -1;
    result[bit+1] <= repeat(count) @(posedge clk) 1'b0;
    #1
    if ($simtime != 72 || result !== 8'bxxxxxx01) begin
      $display("Failed @ at %0t, expected 8'bxxxxxx01, got %h",
               $simtime, result);
      pass = 1'b0;
    end

    result[bit+2] <= repeat(0) @(posedge clk) 1'b1;
    #1
    if ($simtime != 73 || result !== 8'bxxxxx101) begin
      $display("Failed @ at %0t, expected 8'bxxxxx101, got %h",
               $simtime, result);
      pass = 1'b0;
    end

    result[bit+3] <= repeat(-1) @(posedge clk) 1'b0;
    #1
    if ($simtime != 74 || result !== 8'bxxxx0101) begin
      $display("Failed @ at %0t, expected 8'bxxxx0101, got %h",
               $simtime, result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
    $finish;
  end
endmodule
`end_keywords
