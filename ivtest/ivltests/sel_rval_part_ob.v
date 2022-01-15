// Module to test the messages for out of bound R-value part selects.

`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass;
  reg big_param;
  reg [1:0] part;
  integer idx;

  parameter pvar0 = 0;
  parameter pvar1 = 1;
  parameter pvar2 = -1;
  parameter pvar3 = 4'b0001;
  parameter [4:1] pvar4 = 4'b0001;
  parameter [1:4] pvar5 = 4'b0001;
  reg [4:1] rvar = 4'b1000;
  reg [1:4] rvar2 = 4'b1000;
  reg [4:1] ravar [2:1];
  reg [1:4] ravar2 [2:1];
  wire [4:1] wvar = 4'b1010;
  wire [1:4] wvar2 = 4'b1010;
  wire [4:1] wavar [2:1];
  wire [1:4] wavar2 [2:1];

  assign wavar[1] = 4'b0111;
  assign wavar[2] = 4'b1110;
  assign wavar2[1] = 4'b0111;
  assign wavar2[2] = 4'b1110;

  initial begin
    pass = 1'b1;
    ravar[1] = 4'b0111;
    ravar[2] = 4'b1110;
    ravar2[1] = 4'b0111;
    ravar2[2] = 4'b1110;
    #1;

    // Icarus supports an unlimited size for unsized parameters. The
    // following checks the 33rd bit to see if it is 1'bx. If so we
    // assume that the simulator only support 32 bit, otherwise we
    // modify our after check for unsized parameters to work (pass)
    // with a larger constant.
    big_param = 1'b1;
    idx = 32;
    if (pvar0[idx] === 1'bx) big_param = 1'b0;

    // Check a parameter with default size equal to 0.
    part = pvar0[31:30];  // At end
    if (part !== 2'b00) begin
      $display("Failed at end part select of a parameter (0), got %b", part);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = pvar0[33:32];  // May be after all
    if (part !== (big_param ? 2'b00: 2'bxx)) begin
      $display("Failed after part select of a parameter (0), got %b", part);
      pass = 1'b0;
    end
    part = pvar0[32:31];  // May be partial after
    if (part !== (big_param ? 2'b00 : 2'bx0)) begin
      $display("Failed partial after part select of a parameter (0), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar0[-1:-2];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a parameter (0), got %b", part);
      pass = 1'b0;
    end
    part = pvar0[0:-1];  // Partial before
    if (part !== 2'b0x) begin
      $display("Failed partial before part select of a parameter (0), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar0[1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a parameter (0), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar0[1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a parameter (0), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar0[1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a parameter (0), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar0[1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a parameter (0), got %b",
               part);
      pass = 1'b0;
    end
`endif

    // Check a parameter with default size equal to 1.
    part = pvar1[31:30];  // At end
    if (part !== 2'b00) begin
      $display("Failed at end part select of a parameter (1), got %b", part);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = pvar1[33:32];  // May be after all
    if (part !== (big_param ? 2'b00: 2'bxx)) begin
      $display("Failed after part select of a parameter (1), got %b", part);
      pass = 1'b0;
    end
    part = pvar1[32:31];  // May be partial after
    if (part !== (big_param ? 2'b00 : 2'bx0)) begin
      $display("Failed partial after part select of a parameter (1), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar1[-1:-2];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a parameter (1), got %b", part);
      pass = 1'b0;
    end
    part = pvar1[0:-1];  // Partial before
    if (part !== 2'b1x) begin
      $display("Failed partial before part select of a parameter (1), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar1[1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a parameter (1), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar1[1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a parameter (1), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar1[1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a parameter (1), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar1[1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a parameter (1), got %b",
               part);
      pass = 1'b0;
    end
`endif

    // Check a parameter with default size equal to -1.
    part = pvar2[31:30];  // At end
    if (part !== 2'b11) begin
      $display("Failed at end part select of a parameter (2), got %b", part);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = pvar2[33:32];  // May be after all
    if (part !== (big_param ? 2'b11: 2'bxx)) begin
      $display("Failed after part select of a parameter (2), got %b", part);
      pass = 1'b0;
    end
    part = pvar2[32:31];  // May be partial after
    if (part !== (big_param ? 2'b11 : 2'bx1)) begin
      $display("Failed partial after part select of a parameter (2), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar2[-1:-2];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a parameter (2), got %b", part);
      pass = 1'b0;
    end
    part = pvar2[0:-1];  // Partial before
    if (part !== 2'b1x) begin
      $display("Failed partial before part select of a parameter (2), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar2[1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a parameter (2), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar2[1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a parameter (2), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar2[1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a parameter (2), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar2[1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a parameter (2), got %b",
               part);
      pass = 1'b0;
    end
`endif

    // Check a parameter with size four from the value.
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = pvar3[5:4];  // After all
    if (part !== 2'bxx) begin
      $display("Failed after part select of a parameter (3), got %b", part);
      pass = 1'b0;
    end
    part = pvar3[4:3];  // Partial after
    if (part !== 2'bx0) begin
      $display("Failed partial after part select of a parameter (3), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar3[-1:-2];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a parameter (3), got %b", part);
      pass = 1'b0;
    end
    part = pvar3[0:-1];  // Partial before
    if (part !== 2'b1x) begin
      $display("Failed partial before part select of a parameter (3), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar3[1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a parameter (3), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar3[1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a parameter (3), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar3[1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a parameter (3), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar3[1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a parameter (3), got %b",
               part);
      pass = 1'b0;
    end
`endif

    // Check a parameter with size four from the range [4:1].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = pvar4[6:5];  // After all
    if (part !== 2'bxx) begin
      $display("Failed after part select of a parameter (4), got %b", part);
      pass = 1'b0;
    end
    part = pvar4[5:4];  // Partial after
    if (part !== 2'bx0) begin
      $display("Failed partial after part select of a parameter (4), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar4[0:-1];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a parameter (4), got %b", part);
      pass = 1'b0;
    end
    part = pvar4[1:0];  // Partial before
    if (part !== 2'b1x) begin
      $display("Failed partial before part select of a parameter (4), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar4[1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a parameter (4), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar4[1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a parameter (4), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar4[1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a parameter (4), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar4[1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a parameter (4), got %b",
               part);
      pass = 1'b0;
    end
`endif

    // Check a parameter with size four from the range [1:4].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = pvar5[-1:0];  // After all
    if (part !== 2'bxx) begin
      $display("Failed after part select of a parameter (5), got %b", part);
      pass = 1'b0;
    end
    part = pvar5[0:1];  // Partial after
    if (part !== 2'bx0) begin
      $display("Failed partial after part select of a parameter (5), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar5[5:6];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a parameter (5), got %b", part);
      pass = 1'b0;
    end
    part = pvar5[4:5];  // Partial before
    if (part !== 2'b1x) begin
      $display("Failed partial before part select of a parameter (5), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar5[1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a parameter (5), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar5[1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a parameter (5), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar5[1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a parameter (5), got %b",
               part);
      pass = 1'b0;
    end
    part = pvar5[1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a parameter (5), got %b",
               part);
      pass = 1'b0;
    end
`endif


    // Check a register with range [4:1].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = rvar[6:5];  // After all
    if (part !== 2'bxx) begin
      $display("Failed after part select of a register, got %b", part);
      pass = 1'b0;
    end
    part = rvar[5:4];  // Partial after
    if (part !== 2'bx1) begin
      $display("Failed partial after part select of a register, got %b", part);
      pass = 1'b0;
    end
    part = rvar[0:-1];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a register, got %b", part);
      pass = 1'b0;
    end
    part = rvar[1:0];  // Partial before
    if (part !== 2'b0x) begin
      $display("Failed partial before part select of a register, got %b", part);
      pass = 1'b0;
    end
    part = rvar[1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a register, got %b", part);
      pass = 1'b0;
    end
    part = rvar[1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a register, got %b", part);
      pass = 1'b0;
    end
    part = rvar[1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a register, got %b", part);
      pass = 1'b0;
    end
    part = rvar[1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a register, got %b", part);
      pass = 1'b0;
    end
`endif

    // Check a register with range [1:4].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = rvar2[-1:0];  // After all
    if (part !== 2'bxx) begin
      $display("Failed after part select of a register (2), got %b", part);
      pass = 1'b0;
    end
    part = rvar2[0:1];  // Partial after
    if (part !== 2'bx1) begin
      $display("Failed partial after part select of a register (2), got %b",
               part);
      pass = 1'b0;
    end
    part = rvar2[5:6];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a register (2), got %b", part);
      pass = 1'b0;
    end
    part = rvar2[4:5];  // Partial before
    if (part !== 2'b0x) begin
      $display("Failed partial before part select of a register (2), got %b",
               part);
      pass = 1'b0;
    end
    part = rvar2[1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a register (2), got %b",
               part);
      pass = 1'b0;
    end
    part = rvar2[1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a register (2), got %b",
               part);
      pass = 1'b0;
    end
    part = rvar2[1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a register (2), got %b", part);
      pass = 1'b0;
    end
    part = rvar2[1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a register (2), got %b", part);
      pass = 1'b0;
    end
`endif


    // Check an array word with range [4:1].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = ravar[1][6:5];  // After all
    if (part !== 2'bxx) begin
      $display("Failed after part select of an array word, got %b", part);
      pass = 1'b0;
    end
    part = ravar[1][5:4];  // Partial after
    if (part !== 2'bx0) begin
      $display("Failed partial after part select of an array word, got %b",
               part);
      pass = 1'b0;
    end
    part = ravar[1][0:-1];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of an array word, got %b", part);
      pass = 1'b0;
    end
    part = ravar[1][1:0];  // Partial before
    if (part !== 2'b1x) begin
      $display("Failed partial before part select of an array word, got %b",
               part);
      pass = 1'b0;
    end
    part = ravar[1][1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of an array word, got %b",
               part);
      pass = 1'b0;
    end
    part = ravar[1][1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of an array word, got %b",
               part);
      pass = 1'b0;
    end
    part = ravar[1][1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of an array word, got %b", part);
      pass = 1'b0;
    end
    part = ravar[1][1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of an array word, got %b", part);
      pass = 1'b0;
    end
`endif

    // Check an array word with range [1:4].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = ravar2[1][-1:0];  // After all
    if (part !== 2'bxx) begin
      $display("Failed after part select of an array word (2), got %b", part);
      pass = 1'b0;
    end
    part = ravar2[1][0:1];  // Partial after
    if (part !== 2'bx0) begin
      $display("Failed partial after part select of an array word (2), got %b",
               part);
      pass = 1'b0;
    end
    part = ravar2[1][5:6];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of an array word (2), got %b", part);
      pass = 1'b0;
    end
    part = ravar2[1][4:5];  // Partial before
    if (part !== 2'b1x) begin
      $display("Failed partial before part select of an array word (2), got %b",
               part);
      pass = 1'b0;
    end
    part = ravar2[1][1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of an array word (2), got %b",
               part);
      pass = 1'b0;
    end
    part = ravar2[1][1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of an array word (2), got %b",
               part);
      pass = 1'b0;
    end
    part = ravar2[1][1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of an array word (2), got %b",
               part);
      pass = 1'b0;
    end
    part = ravar2[1][1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of an array word (2), got %b",
               part);
      pass = 1'b0;
    end
`endif


    // Check a wire with range [4:1].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = wvar[6:5];  // After all
    if (part !== 2'bxx) begin
      $display("Failed after part select of a wire, got %b", part);
      pass = 1'b0;
    end
    part = wvar[5:4];  // Partial after
    if (part !== 2'bx1) begin
      $display("Failed partial after part select of a wire, got %b", part);
      pass = 1'b0;
    end
    part = wvar[0:-1];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a wire, got %b", part);
      pass = 1'b0;
    end
    part = wvar[1:0];  // Partial before
    if (part !== 2'b0x) begin
      $display("Failed partial before part select of a wire, got %b", part);
      pass = 1'b0;
    end
    part = wvar[1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a wire, got %b", part);
      pass = 1'b0;
    end
    part = wvar[1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a wire, got %b", part);
      pass = 1'b0;
    end
    part = wvar[1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a wire, got %b", part);
      pass = 1'b0;
    end
    part = wvar[1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a wire, got %b", part);
      pass = 1'b0;
    end
`endif

    // Check a wire with range [1:4].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = wvar2[-1:0];  // After all
    if (part !== 2'bxx) begin
      $display("Failed after part select of a wire (2), got %b", part);
      pass = 1'b0;
    end
    part = wvar2[0:1];  // Partial after
    if (part !== 2'bx1) begin
      $display("Failed partial after part select of a wire (2), got %b", part);
      pass = 1'b0;
    end
    part = wvar2[5:6];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a wire (2), got %b", part);
      pass = 1'b0;
    end
    part = wvar2[4:5];  // Partial before
    if (part !== 2'b0x) begin
      $display("Failed partial before part select of a wire (2), got %b", part);
      pass = 1'b0;
    end
    part = wvar2[1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a wire (2), got %b", part);
      pass = 1'b0;
    end
    part = wvar2[1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a wire (2), got %b", part);
      pass = 1'b0;
    end
    part = wvar2[1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a wire (2), got %b", part);
      pass = 1'b0;
    end
    part = wvar2[1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a wire (2), got %b", part);
      pass = 1'b0;
    end
`endif


    // Check a wire array word with range [4:1].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = wavar[1][6:5];  // After all
    if (part !== 2'bxx) begin
      $display("Failed after part select of a wire array word, got %b", part);
      pass = 1'b0;
    end
    part = wavar[1][5:4];  // Partial after
    if (part !== 2'bx0) begin
      $display("Failed partial after part select of a wire array word, got %b",
               part);
      pass = 1'b0;
    end
    part = wavar[1][0:-1];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a wire array word, got %b", part);
      pass = 1'b0;
    end
    part = wavar[1][1:0];  // Partial before
    if (part !== 2'b1x) begin
      $display("Failed partial before part select of a wire array word, got %b",
               part);
      pass = 1'b0;
    end
    part = wavar[1][1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a wire array word, got %b",
               part);
      pass = 1'b0;
    end
    part = wavar[1][1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a wire array word, got %b",
               part);
      pass = 1'b0;
    end
    part = wavar[1][1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a wire array word, got %b",
               part);
      pass = 1'b0;
    end
    part = wavar[1][1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a wire array word, got %b",
               part);
      pass = 1'b0;
    end
`endif

    // Check a wire array word with range [1:4].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    part = wavar2[1][-1:0];  // After all
    if (part !== 2'bxx) begin
      $display("Failed after part select of a wire array word (2), got %b",
               part);
      pass = 1'b0;
    end
    part = wavar2[1][0:1];  // Partial after
    if (part !== 2'bx0) begin
      $display("Failed partial after part select of a wire array word (2),",
               " got %b", part);
      pass = 1'b0;
    end
    part = wavar2[1][5:6];  // Before all
    if (part !== 2'bxx) begin
      $display("Failed before part select of a wire array word (2), got %b",
               part);
      pass = 1'b0;
    end
    part = wavar2[1][4:5];  // Partial before
    if (part !== 2'b1x) begin
      $display("Failed partial before part select of a wire array word (2),",
               " got %b", part);
      pass = 1'b0;
    end
    part = wavar2[1][1'bx:1];  // Undefined 1st
    if (part !== 2'bxx) begin
      $display("Failed undefined 1st part select of a wire array word (2),",
               " got %b", part);
      pass = 1'b0;
    end
    part = wavar2[1][1:1'bx];  // Undefined 2nd
    if (part !== 2'bxx) begin
      $display("Failed undefined 2nd part select of a wire array word (2),",
               " got %b", part);
      pass = 1'b0;
    end
    part = wavar2[1][1'bz:1];  // High-Z 1st
    if (part !== 2'bxx) begin
      $display("Failed high-Z 1st part select of a wire array word (2), got %b",
               part);
      pass = 1'b0;
    end
    part = wavar2[1][1:1'bz];  // High-Z 2nd
    if (part !== 2'bxx) begin
      $display("Failed high-Z 2nd part select of a wire array word (2), got %b",
               part);
      pass = 1'b0;
    end
`endif

    if (pass) $display("PASSED");
  end
endmodule
