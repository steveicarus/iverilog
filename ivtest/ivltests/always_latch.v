module top;
  reg y, a, b, flip, hidden, en;
  reg pass;

  function f_and (input i1, i2);
    reg  partial;
    begin
      partial = i1 & i2;
      f_and = partial | hidden;
    end
  endfunction

  reg intr;
  always_latch begin
    if (en) begin
      intr = flip;
      y <= f_and(a, b) ^ intr;
    end
  end

  initial begin
    pass = 1'b1;

    en = 1'b1;
    flip = 1'b0;
    hidden = 1'b0;
    a = 1'b0;
    b = 1'b0;
    #1;
    if (y !== 1'b0) begin
      $display("FAILED: a=1'b0, b=1'b0, hidden=1'b0, expected 1'b0, got %b", y);
      pass = 1'b0;
    end

    a = 1'b0;
    b = 1'b1;
    #1;
    if (y !== 1'b0) begin
      $display("FAILED: a=1'b0, b=1'b1, hidden=1'b0, expected 1'b0, got %b", y);
      pass = 1'b0;
    end

    a = 1'b1;
    b = 1'b0;
    #1;
    if (y !== 1'b0) begin
      $display("FAILED: a=1'b1, b=1'b0, hidden=1'b0, expected 1'b0, got %b", y);
      pass = 1'b0;
    end

    a = 1'b1;
    b = 1'b1;
    #1;
    if (y !== 1'b1) begin
      $display("FAILED: a=1'b1, b=1'b1, hidden=1'b0, expected 1'b1, got %b", y);
      pass = 1'b0;
    end

    hidden = 1'b0;
    a = 1'b0;
    b = 1'b0;
    #1;
    if (y !== 1'b0) begin
      $display("FAILED: a=1'b0, b=1'b0, hidden=1'b0, expected 1'b0, got %b", y);
      pass = 1'b0;
    end

    hidden = 1'b1;
    a = 1'b0;
    b = 1'b0;
    #1;
    if (y !== 1'b1) begin
      $display("FAILED: a=1'b0, b=1'b0, hidden=1'b1, expected 1'b1, got %b", y);
      pass = 1'b0;
    end

    en = 1'b0;
    hidden = 1'b0;
    #1;
    if (y !== 1'b1) begin
      $display("FAILED: en=1'b0, expected 1'b1, got %b", y);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
