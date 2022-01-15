module top;
  reg pass = 1'b1;

  integer delay;
  reg [3:0] in = 4'h0;
  reg [7:0] result [1:0];

  initial begin
    result[0] <= #10 8'h00;
    if ($simtime != 0 || result[0] !== 8'bx) begin
      $display("Failed #10 blocked at %0t, expected 8'hxx, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end
    @(result[0]);
    if ($simtime != 10 || result[0] !== 8'h00) begin
      $display("Failed #10 at %0t, expected 8'h00, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end

    result[0][8:5] <= #10 4'hb;
    @(result[0]);
    if ($simtime != 20 || result[0] !== 8'h60) begin
      $display("Failed MSB #10 at %0t, expected 8'h60, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end

    result[0][1:-2] <= #10 4'hb;
    @(result[0]);
    if ($simtime != 30 || result[0] !== 8'h62) begin
      $display("Failed LSB #10 at %0t, expected 8'h62, got %h",
               $simtime, result[0]);
      pass = 1'b0;
    end

    delay = 20;
    result[1] <= #delay 8'h00;
    if ($simtime != 30 || result[1] !== 8'bx) begin
      $display("Failed #delay blocked at %0t, expected 8'hxx, got %h",
               $simtime, result[1]);
      pass = 1'b0;
    end
    @(result[1]);
    if ($simtime != 50 || result[1] !== 8'h00) begin
      $display("Failed #delay at %0t, expected 8'h00, got %h",
               $simtime, result[1]);
      pass = 1'b0;
    end

    result[1][8:5] <= #delay 4'hb;
    @(result[1]);
    if ($simtime != 70 || result[1] !== 8'h60) begin
      $display("Failed MSB #delay at %0t, expected 8'h60, got %h",
               $simtime, result[1]);
      pass = 1'b0;
    end

    result[1][1:-2] <= #delay 4'hb;
    @(result[1]);
    if ($simtime != 90 || result[1] !== 8'h62) begin
      $display("Failed LSB #delay at %0t, expected 8'h62, got %h",
               $simtime, result[1]);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
    $finish;
  end
endmodule
