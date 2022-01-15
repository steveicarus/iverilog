module test;
  reg fail = 0;
  reg [3:0] in = 4'b0;
  wire [3:0] bus = in;

  initial begin
    #1;  // Need some delay for the calculations to run.
    if (bus !== 4'b0) begin
      $display("FAILED: initial value, got %b, expected 0000.", bus);
      fail = 1;
    end

    #1 force bus[0] = 1'b1;
    #1 in[0] = 1'bz;
    if (bus !== 4'b0001) begin
      $display("FAILED: force of bus[0], got %b, expected 0001.", bus);
      fail = 1;
    end

    #1 force bus[3:2] = 2'b11;
    if (bus !== 4'b1101) begin
      $display("FAILED: force of bus[3:2], got %b, expected 1101.", bus);
      fail = 1;
    end

    #1 release bus[0];
    if (bus !== 4'b110z) begin
      $display("FAILED: release of bus[0], got %b, expected 110z.", bus);
      fail = 1;
    end

    #1 release bus[3:2];
    if (bus !== 4'b000z) begin
      $display("FAILED: release of bus[3:2], got %b, expected 000z.", bus);
      fail = 1;
    end

    if (!fail) $display("PASSED");
  end
endmodule
