module top;
  reg  pass;
  wire out;
  reg  drive_val;
  reg  oe_n;
  reg [1:0] pull_vec;

  bufif0 (out, drive_val, oe_n);
  assign (pull0, pull1) out = pull_vec[0];

  initial begin
    pass = 1'b1;
    pull_vec = 2'b00;
    oe_n      = 1'b0;

    // Drive is selected.
    drive_val = 1'b0;
    #1;
    if (out !== drive_val) begin
      $display("Failed to drive 0, got %b", out);
      pass = 1'b0;
    end

    drive_val = 1'b1;
    #1;
    if (out !== drive_val) begin
      $display("Failed to drive 1, got %b", out);
      pass = 1'b0;
    end

    // The pull is selected (low).
    oe_n      = 1'b1;
    drive_val = 1'b0;
    #1;
    if (out !== pull_vec[0]) begin
      $display("Failed pull #1, expected 1'b0, got %b", out);
      pass = 1'b0;
    end

    drive_val = 1'b1;
    #1;
    if (out !== pull_vec[0]) begin
      $display("Failed pull #2, expected 1'b0, got %b", out);
      pass = 1'b0;
    end

    // The pull is selected (high).
    pull_vec = 2'b11;
    drive_val = 1'b0;
    #1;
    if (out !== pull_vec[0]) begin
      $display("Failed pull #3, expected 1'b1, got %b", out);
      pass = 1'b0;
    end

    drive_val = 1'b1;
    if (out !== pull_vec[0]) begin
      $display("Failed pull #4, expected 1'b1, got %b", out);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
