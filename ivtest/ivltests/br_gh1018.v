module top;
  // Test the shortreal conversion functions, but we don't support shortreal
  // as a type yet so be safe with the actual values that are checked to
  // avoid any real (double) to shortreal (float) rounding.
  real in;
  reg [31:0] irep;
  real res;
  reg passed;

  initial begin
    passed = 1'b1;

    in = 0.0;
    irep = $shortrealtobits(in);
    res  = $bitstoshortreal(irep);
    if (in != res) begin
      $display("Input and output value do not match (%f != %f)", in, res);
      passed = 1'b0;
    end

    in = 8.0;
    irep = $shortrealtobits(in);
    res  = $bitstoshortreal(irep);
    if (in != res) begin
      $display("Input and output value do not match (%f != %f)", in, res);
      passed = 1'b0;
    end


    in = 0.125;
    irep = $shortrealtobits(in);
    res  = $bitstoshortreal(irep);
    if (in != res) begin
      $display("Input and output value do not match (%f != %f)", in, res);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
