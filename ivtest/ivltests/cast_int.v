module top;
  reg pass = 1'b1;
  real in, bin;
  wire [7:0] out = in;
  wire signed [34:0] big = bin;

  initial begin
//    $monitor(in,, out,, bin,, big);
    bin = 8589934592.5; // 2**33+0.5 overflows a 32 bit long.
    #1;
    if (big !== 35'sd8589934593) begin
      $display("Failed: multiword check, expected 8589934593, got %d", big);
      pass = 1'b0;
    end

    if (out !== 'b0) begin
      $display("Failed: initial value, expected 8'b0, got %b", out);
      pass = 1'b0;
    end

    in = 0.499999;
    bin = -25.5; // This test a different branch (small result -> big vec.).
    #1;
    if (big !== -26) begin
      $display("Failed: small value multiword check, expected -26, got %d", out);
      pass = 1'b0;
    end

    if (out !== 8'b0) begin
      $display("Failed: rounding value (down, +), expected 8'b0, got %b", out);
      pass = 1'b0;
    end

    in = -0.499999;
    #1;
    if (out !== 8'b0) begin
      $display("Failed: rounding value (down, -), expected 8'b0, got %b", out);
      pass = 1'b0;
    end

    in = 0.5;
    #1;
    if (out !== 8'b01) begin
      $display("Failed: rounding value (up, +), expected 8'b01, got %b", out);
      pass = 1'b0;
    end

    in = -0.5;
    #1;
    if (out !== 8'b11111111) begin
      $display("Failed: rounding value (up, -), expected 8'b11111111, got %b", out);
      pass = 1'b0;
    end

    in = 256.0;
    #1;
    if (out !== 8'b0) begin
      $display("Failed: overflow expected 8'b0, got %b", out);
      pass = 1'b0;
    end

    in = 511.0;
    #1;
    if (out !== 8'b11111111) begin
      $display("Failed: pruning expected 8'b11111111, got %b", out);
      pass = 1'b0;
    end

    in = 1.0/0.0;
    #1;
    if (out !== 8'bxxxxxxxx) begin
      $display("Failed: +inf expected 8'bxxxxxxxx, got %b", out);
      pass = 1'b0;
    end

    in = -1.0/0.0;
    #1;
    if (out !== 8'bxxxxxxxx) begin
      $display("Failed: -inf expected 8'bxxxxxxxx, got %b", out);
      pass = 1'b0;
    end

    in = $sqrt(-1.0);
    #1;
    if (out !== 8'bxxxxxxxx) begin
      $display("Failed: nan expected 8'bxxxxxxxx, got %b", out);
      pass = 1'b0;
    end

    in = 8589934720.5;
    #1;
    if (out !== 129) begin
      $display("Failed: overflow value expected 129, got %d", out);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
