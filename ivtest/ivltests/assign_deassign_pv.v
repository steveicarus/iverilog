module test;
  reg fail = 1'b0;
  reg [3:0] bus = 4'b0;

  initial begin
    // Check the initial value.
    if (bus !== 4'b0) begin
      $display("FAILED: initial value, got %b, expected 0000.", bus);
      fail = 1'b1;
    end

    // Check a bit assign and verify a normal bit assign does nothing.
    #1 assign bus[0] = 1'b1;
    bus[0] = 1'bz;
    if (bus !== 4'b0001) begin
      $display("FAILED: assign of bus[0], got %b, expected 0001.", bus);
      fail = 1'b1;
    end

    // Check a part assign.
    #1 assign bus[3:2] = 2'b11;
    if (bus !== 4'b1101) begin
      $display("FAILED: assign of bus[3:2], got %b, expected 1101.", bus);
      fail = 1'b1;
    end

    // Check that we can change an unassigned bit.
    #1 bus[1] = 1'bz;
    if (bus !== 4'b11z1) begin
      $display("FAILED: assignment of bus[1], got %b, expected 11z1.", bus);
      fail = 1'b1;
    end

    // Check a bit deassign.
    #1 deassign bus[0];
    bus = 4'b000z;
    if (bus !== 4'b110z) begin
      $display("FAILED: deassign of bus[0], got %b, expected 110z.", bus);
      fail = 1'b1;
    end

    // Check a part deassign (we keep the old value if not changed).
    #1 deassign bus[3:2];
    bus[3] = 1'b0;
    if (bus !== 4'b010z) begin
      $display("FAILED: deassign of bus[3:2], got %b, expected 010z.", bus);
      fail = 1'b1;
    end

    // Check an assign from the upper thread bits >= 8.
    #1 assign bus[2:1] = 2'bx1;
    if (bus !== 4'b0x1z) begin
      $display("FAILED: assign of bus[2:1], got %b, expected 0x1z.", bus);
      fail = 1'b1;
    end

    if (!fail) $display("PASSED");
  end
endmodule
