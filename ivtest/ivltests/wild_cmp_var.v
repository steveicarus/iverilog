module top;
  reg [1:0] lv, rv;
  reg res, pass;

  initial begin
    pass = 1'b1;

    lv = 2'b00;
    rv = 2'b00;
    res = lv ==? rv;
    if (res !== 1'b1) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b00;
    rv = 2'b01;
    res = lv ==? rv;
    if (res !== 1'b0) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b0", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b10;
    rv = 2'b00;
    res = lv ==? rv;
    if (res !== 1'b0) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b0", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b1x;
    rv = 2'b00;
    res = lv ==? rv;
    if (res !== 1'b0) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b0", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b0x;
    rv = 2'b00;
    res = lv ==? rv;
    if (res !== 1'bx) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'bx", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b00;
    rv = 2'b0x;
    res = lv ==? rv;
    if (res !== 1'b1) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b01;
    rv = 2'b0x;
    res = lv ==? rv;
    if (res !== 1'b1) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b0z;
    rv = 2'b0x;
    res = lv ==? rv;
    if (res !== 1'b1) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b0x;
    rv = 2'b0x;
    res = lv ==? rv;
    if (res !== 1'b1) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b00;
    rv = 2'b00;
    res = lv !=? rv;
    if (res !== 1'b0) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b0", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b00;
    rv = 2'b01;
    res = lv !=? rv;
    if (res !== 1'b1) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b10;
    rv = 2'b00;
    res = lv !=? rv;
    if (res !== 1'b1) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b1x;
    rv = 2'b00;
    res = lv !=? rv;
    if (res !== 1'b1) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b0x;
    rv = 2'b00;
    res = lv !=? rv;
    if (res !== 1'bx) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'bx", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b00;
    rv = 2'b0x;
    res = lv !=? rv;
    if (res !== 1'b0) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b0", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b01;
    rv = 2'b0x;
    res = lv !=? rv;
    if (res !== 1'b0) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b0", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b0z;
    rv = 2'b0x;
    res = lv !=? rv;
    if (res !== 1'b0) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b0", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b0x;
    rv = 2'b0x;
    res = lv !=? rv;
    if (res !== 1'b0) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b0", lv, rv, res);
      pass = 1'b0;
    end

    // Check in a few other contexts.

    lv = 2'b01;
    rv = 2'b0x;
    res = (lv ==? rv) ? 1'b1 : 1'b0;
    if (res !== 1'b1) begin
      $display("Failed: %b ==? %b (ternary) returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end

    if (lv !=? rv) begin
      $display("Failed: %b ==? %b (if) returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end

    lv = 2'b00;
    while (lv ==? rv) lv += 2'b01;
    if (lv !== 2'b10) begin
      $display("Failed: %b ==? %b (while) expected lv to be 2'b10", lv, rv);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
