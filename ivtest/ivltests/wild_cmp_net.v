module top;
  reg [1:0] lv, rv;
  reg pass;
  wire res, resb;

  assign res = lv  ==? rv;
  assign resb = lv  !=? rv;

  initial begin
    pass = 1'b1;

    lv = 2'b00;
    rv = 2'b00;
    #1;
    if (res !== 1'b1) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end

    if (resb !== 1'b0) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b0", lv, rv, resb);
      pass = 1'b0;
    end

    #1;
    lv = 2'b00;
    rv = 2'b01;
    #1;
    if (res !== 1'b0) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b0", lv, rv, res);
      pass = 1'b0;
    end
    if (resb !== 1'b1) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b1", lv, rv, resb);
      pass = 1'b0;
    end

    #1;
    lv = 2'b10;
    rv = 2'b00;
    #1;
    if (res !== 1'b0) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b0", lv, rv, res);
      pass = 1'b0;
    end
    if (resb !== 1'b1) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b1", lv, rv, resb);
      pass = 1'b0;
    end

    #1;
    lv = 2'b1x;
    rv = 2'b00;
    #1;
    if (res !== 1'b0) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b0", lv, rv, res);
      pass = 1'b0;
    end
    if (resb !== 1'b1) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b1", lv, rv, resb);
      pass = 1'b0;
    end

    #1;
    lv = 2'b0x;
    rv = 2'b00;
    #1;
    if (res !== 1'bx) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'bx", lv, rv, res);
      pass = 1'b0;
    end
    if (resb !== 1'bx) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'bx", lv, rv, resb);
      pass = 1'b0;
    end

    #1;
    lv = 2'b00;
    rv = 2'b0x;
    #1;
    if (res !== 1'b1) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end
    if (resb !== 1'b0) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b0", lv, rv, resb);
      pass = 1'b0;
    end

    #1;
    lv = 2'b01;
    rv = 2'b0x;
    #1;
    if (res !== 1'b1) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end
    if (resb !== 1'b0) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b0", lv, rv, resb);
      pass = 1'b0;
    end

    #1;
    lv = 2'b0z;
    rv = 2'b0x;
    #1;
    if (res !== 1'b1) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end
    if (resb !== 1'b0) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b0", lv, rv, resb);
      pass = 1'b0;
    end

    #1;
    lv = 2'b0x;
    rv = 2'b0x;
    #1;
    if (res !== 1'b1) begin
      $display("Failed: %b ==? %b returned 1'b%b not 1'b1", lv, rv, res);
      pass = 1'b0;
    end
    if (resb !== 1'b0) begin
      $display("Failed: %b !=? %b returned 1'b%b not 1'b0", lv, rv, resb);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
