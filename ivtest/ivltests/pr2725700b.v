module top;
  reg pass;
  wire [1:0] out;
  reg  [1:0] drive_val;
  reg  [1:0] oe_n;
  reg  [2:0] pull_vec;

  bufif0 bufs[1:0] (out, drive_val, oe_n);
  assign (pull0, pull1) out = pull_vec[1:0];

  initial begin
    pass = 1'b1;
    pull_vec = 3'b000;
    oe_n      = 2'b00;

    // Drive is selected.
    drive_val = 2'b00;
    #1;
    if (out !== drive_val) begin
      $display("Failed to drive 2'b00, got %b", out);
      pass = 1'b0;
    end

    drive_val = 1'b1;
    #1;
    if (out !== drive_val) begin
      $display("Failed to drive 2'b11, got %b", out);
      pass = 1'b0;
    end

    // The pull is selected (low).
    oe_n      = 2'b11;
    drive_val = 2'b00;
    #1;
    if (out !== pull_vec[1:0]) begin
      $display("Failed pull #1, expected 2'b00, got %b", out);
      pass = 1'b0;
    end

    drive_val = 1'b1;
    #1;
    if (out !== pull_vec[1:0]) begin
      $display("Failed pull #2, expected 2'b00, got %b", out);
      pass = 1'b0;
    end

    // The pull is selected (high).
    pull_vec = 3'b111;
    drive_val = 2'b00;
    #1;
    if (out !== pull_vec[1:0]) begin
      $display("Failed pull #3, expected 2'b11, got %b", out);
      pass = 1'b0;
    end

    drive_val = 2'b11;
    if (out !== pull_vec[1:0]) begin
      $display("Failed pull #4, expected 2'b11, got %b", out);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
