// Check the power operator (run time).
module top;
  reg pass;

  integer res;
  reg signed [31:0] l, r;

  initial begin
    pass = 1'b1;

    // Check the constant ** with various arguments (table 5-6 1364-2005).

    l = -3;
    r = 'bx;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant -3**'bx, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    l = -1;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant -1**'bx, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    l = 0;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 0**'bx, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    l = 1;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 1**'bx, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    l = 3;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 3**'bx, expected 'bx, got %0d", res);
      pass = 1'b0;
    end

    l = 'bx;
    r=-3;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**-3, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    r=-2;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**-2, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    r=-1;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**-1, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    r=0;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**0, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    r=1;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**1, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    r=2;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**2, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    r=3;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**3, expected 'bx, got %0d", res);
      pass = 1'b0;
    end

    // Check the 1st line (rvalue is positive).
    l=-3;
    r=3;
    res = l**r;
    if (res !== -27) begin
      $display("Failed: constant -3**3, expected -27, got %0d", res);
      pass = 1'b0;
    end
    l=-3;
    r=2;
    res = l**r;
    if (res !== 9) begin
      $display("Failed: constant -3**2, expected 9, got %0d", res);
      pass = 1'b0;
    end

    l=-1;
    r=3;
    res = l**r;
    if (res !== -1) begin
      $display("Failed: constant -1**3, expected -1, got %0d", res);
      pass = 1'b0;
    end
    l=-1;
    r=2;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant -1**2, expected 1, got %0d", res);
      pass = 1'b0;
    end

    l=0;
    r=3;
    res = l**r;
    if (res !== 0) begin
      $display("Failed: constant 0**3, expected 0, got %0d", res);
      pass = 1'b0;
    end
    l=0;
    r=2;
    res = l**r;
    if (res !== 0) begin
      $display("Failed: constant 0**2, expected 0, got %0d", res);
      pass = 1'b0;
    end

    l=1;
    r=3;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant 1**3, expected 1, got %0d", res);
      pass = 1'b0;
    end
    l=1;
    r=2;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant 1**2, expected 1, got %0d", res);
      pass = 1'b0;
    end

    l=3;
    r=3;
    res = l**r;
    if (res !== 27) begin
      $display("Failed: constant 3**3, expected 27, got %0d", res);
      pass = 1'b0;
    end
    l=3;
    r=2;
    res = l**r;
    if (res !== 9) begin
      $display("Failed: constant 3**2, expected 9, got %0d", res);
      pass = 1'b0;
    end

    // Check the 2nd line (rvalue is zero).
    l=-3;
    r=0;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant -3**0, expected 1, got %0d", res);
      pass = 1'b0;
    end
    l=-2;
    r=0;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant -2**0, expected 1, got %0d", res);
      pass = 1'b0;
    end

    l=-1;
    r=0;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant -1**0, expected 1, got %0d", res);
      pass = 1'b0;
    end

    l=0;
    r=0;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant 0**0, expected 1, got %0d", res);
      pass = 1'b0;
    end

    l=1;
    r=0;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant 1**0, expected 1, got %0d", res);
      pass = 1'b0;
    end

    l=2;
    r=0;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant 2**0, expected 1, got %0d", res);
      pass = 1'b0;
    end
    l=3;
    r=0;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant 3**0, expected 1, got %0d", res);
      pass = 1'b0;
    end

    // Check the 3rd line (rvalue is negative).
    l=-2;
    r=-3;
    res = l**r;
    if (res !== 0) begin
      $display("Failed: constant -2**-3, expected 0, got %0d", res);
      pass = 1'b0;
    end
    l=-2;
    r=-2;
    res = l**r;
    if (res !== 0) begin
      $display("Failed: constant -2**-2, expected 0, got %0d", res);
      pass = 1'b0;
    end

    l=-1;
    r=-3;
    res = l**r;
    if (res !== -1) begin
      $display("Failed: constant -1**-3, expected -1, got %0d", res);
      pass = 1'b0;
    end
    l=-1;
    r=-2;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant -1**-2, expected 1, got %0d", res);
      pass = 1'b0;
    end

    l=0;
    r=-3;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 0**-3, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    l=0;
    r=-2;
    res = l**r;
    if (res !== 'bx) begin
      $display("Failed: constant 0**-2, expected 'bx, got %0d", res);
      pass = 1'b0;
    end

    l=1;
    r=-3;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant 1**-3, expected 1, got %0d", res);
      pass = 1'b0;
    end
    l=1;
    r=-2;
    res = l**r;
    if (res !== 1) begin
      $display("Failed: constant 1**-2, expected 1, got %0d", res);
      pass = 1'b0;
    end

    l=2;
    r=-3;
    res = l**r;
    if (res !== 0) begin
      $display("Failed: constant 2**-3, expected 0, got %0d", res);
      pass = 1'b0;
    end
    l=2;
    r=-2;
    res = l**r;
    if (res !== 0) begin
      $display("Failed: constant 2**-2, expected 0, got %0d", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
