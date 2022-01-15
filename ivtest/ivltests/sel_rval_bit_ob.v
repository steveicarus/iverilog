`begin_keywords "1364-2005"
// Module to test the messages/results for out of bound R-value constant
// bit selects.

`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass;
  reg big_param;
  reg bit;

  integer idx;

  parameter pvar0 = 0;
  parameter pvar1 = 1;
  parameter pvar2 = -1;
  parameter pvar3 = 4'b0001;
  parameter [4:1] pvar4 = 4'b0001;
  parameter [1:4] pvar5 = 4'b0001;
  reg [4:1] rvar = 4'b0010;
  reg [1:4] rvar2 = 4'b0010;
  reg [4:1] ravar [2:1];
  reg [1:4] ravar2 [2:1];
  wire [4:1] wvar = 4'b0100;
  wire [1:4] wvar2 = 4'b0100;
  wire [4:1] wavar [2:1];
  wire [1:4] wavar2 [2:1];

  assign wavar[1] = 4'b1001;
  assign wavar[2] = 4'b1010;
  assign wavar2[1] = 4'b1001;
  assign wavar2[2] = 4'b1010;

  initial begin
    pass = 1'b1;
    ravar[1] = 4'b1101;
    ravar[2] = 4'b1110;
    ravar2[1] = 4'b1101;
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
    bit = pvar0[31];  // At end
    if (bit !== 1'b0) begin
      $display("Failed at end bit select of a parameter (0), got %b", bit);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = pvar0[32];  // May be after
    if (bit !== (big_param ? 1'b0: 1'bx)) begin
      $display("Failed after bit select of a parameter (0), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar0[-1];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a parameter (0), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar0[1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a parameter (0), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar0[1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a parameter (0), got %b", bit);
      pass = 1'b0;
    end
`endif

    // Check a parameter with default size equal to 1.
    bit = pvar1[31];  // At end
    if (bit !== 1'b0) begin
      $display("Failed at end bit select of a parameter (1), got %b", bit);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = pvar1[32];  // May be after
    if (bit !== (big_param ? 1'b0: 1'bx)) begin
      $display("Failed after bit select of a parameter (1), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar1[-1];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a parameter (1), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar1[1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a parameter (1), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar1[1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a parameter (1), got %b", bit);
      pass = 1'b0;
    end
`endif

    // Check a parameter with default size equal to -1.
    bit = pvar2[31];  // At end
    if (bit !== 1'b1) begin
      $display("Failed at end bit select of a parameter (-1), got %b", bit);
      pass = 1'b0;
    end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = pvar2[32];  // May be after
    if (bit !== (big_param ? 1'b1: 1'bx)) begin
      $display("Failed after bit select of a parameter (-1), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar2[-1];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a parameter (-1), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar2[1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a parameter (-1), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar2[1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a parameter (-1), got %b", bit);
      pass = 1'b0;
    end
`endif

    // Check a parameter with size four from the value.
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = pvar3[4];  // After
    if (bit !== 1'bx) begin
      $display("Failed after bit select of a parameter (3), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar3[-1];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a parameter (3), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar3[1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a parameter (3), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar3[1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a parameter (3), got %b", bit);
      pass = 1'b0;
    end
`endif

    // Check a parameter with size four from the range [4:1].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = pvar4[5];  // After
    if (bit !== 1'bx) begin
      $display("Failed after bit select of a parameter (4), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar4[0];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a parameter (4), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar4[1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a parameter (4), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar4[1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a parameter (4), got %b", bit);
      pass = 1'b0;
    end
`endif

    // Check a parameter with size four from the range [1:4].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = pvar5[0];  // After
    if (bit !== 1'bx) begin
      $display("Failed after bit select of a parameter (5), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar5[5];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a parameter (5), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar5[1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a parameter (5), got %b", bit);
      pass = 1'b0;
    end
    bit = pvar5[1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a parameter (5), got %b", bit);
      pass = 1'b0;
    end
`endif


    // Check a register with range [4:1].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = rvar[5];  // After
    if (bit !== 1'bx) begin
      $display("Failed after bit select of a register, got %b", bit);
      pass = 1'b0;
    end
    bit = rvar[0];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a register, got %b", bit);
      pass = 1'b0;
    end
    bit = rvar[1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a register, got %b", bit);
      pass = 1'b0;
    end
    bit = rvar[1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a register, got %b", bit);
      pass = 1'b0;
    end
`endif

    // Check a register with range [1:4].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = rvar2[0];  // After
    if (bit !== 1'bx) begin
      $display("Failed after bit select of a register (2), got %b", bit);
      pass = 1'b0;
    end
    bit = rvar2[5];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a register (2), got %b", bit);
      pass = 1'b0;
    end
    bit = rvar2[1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a register (2), got %b", bit);
      pass = 1'b0;
    end
    bit = rvar2[1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a register (2), got %b", bit);
      pass = 1'b0;
    end
`endif


    // Check an array word with range [4:1].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = ravar[1][5];  // After
    if (bit !== 1'bx) begin
      $display("Failed after bit select of an array word, got %b", bit);
      pass = 1'b0;
    end
    bit = ravar[1][0];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of an array word, got %b", bit);
      pass = 1'b0;
    end
    bit = ravar[1][1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of an array word, got %b", bit);
      pass = 1'b0;
    end
    bit = ravar[1][1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of an array word, got %b", bit);
      pass = 1'b0;
    end
`endif

    // Check an array word with range [1:4].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = ravar2[1][0];  // After
    if (bit !== 1'bx) begin
      $display("Failed after bit select of an array word (2), got %b", bit);
      pass = 1'b0;
    end
    bit = ravar2[1][5];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of an array word (2), got %b", bit);
      pass = 1'b0;
    end
    bit = ravar2[1][1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of an array word (2), got %b", bit);
      pass = 1'b0;
    end
    bit = ravar2[1][1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of an array word (2), got %b", bit);
      pass = 1'b0;
    end
`endif


    // Check a wire with range [4:1].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = wvar[5];  // After
    if (bit !== 1'bx) begin
      $display("Failed after bit select of a wire, got %b", bit);
      pass = 1'b0;
    end
    bit = wvar[0];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a wire, got %b", bit);
      pass = 1'b0;
    end
    bit = wvar[1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a wire, got %b", bit);
      pass = 1'b0;
    end
    bit = wvar[1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a wire, got %b", bit);
      pass = 1'b0;
    end
`endif

    // Check a wire with range [1:4].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = wvar2[0];  // After
    if (bit !== 1'bx) begin
      $display("Failed after bit select of a wire (2), got %b", bit);
      pass = 1'b0;
    end
    bit = wvar2[5];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a wire (2), got %b", bit);
      pass = 1'b0;
    end
    bit = wvar2[1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a wire (2), got %b", bit);
      pass = 1'b0;
    end
    bit = wvar2[1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a wire (2), got %b", bit);
      pass = 1'b0;
    end
`endif


    // Check a wire array word with range [4:1].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = wavar[1][5];  // After
    if (bit !== 1'bx) begin
      $display("Failed after bit select of a wire array word, got %b", bit);
      pass = 1'b0;
    end
    bit = wavar[1][0];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a wire array word, got %b", bit);
      pass = 1'b0;
    end
    bit = wavar[1][1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a wire array word, got %b", bit);
      pass = 1'b0;
    end
    bit = wavar[1][1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a wire array word, got %b", bit);
      pass = 1'b0;
    end
`endif

    // Check a wire array word with range [1:4].
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    bit = wavar2[1][0];  // After
    if (bit !== 1'bx) begin
      $display("Failed after bit select of a wire array word (2), got %b", bit);
      pass = 1'b0;
    end
    bit = wavar2[1][5];  // Before
    if (bit !== 1'bx) begin
      $display("Failed before bit select of a wire array word (2), got %b",
               bit);
      pass = 1'b0;
    end
    bit = wavar2[1][1'bx];  // Undefined
    if (bit !== 1'bx) begin
      $display("Failed undefined bit select of a wire array word (2), got %b",
               bit);
      pass = 1'b0;
    end
    bit = wavar2[1][1'bz];  // High-Z
    if (bit !== 1'bx) begin
      $display("Failed high-Z bit select of a wire array word (2), got %b",
               bit);
      pass = 1'b0;
    end
`endif

    if (pass) $display("PASSED");
  end
endmodule
`end_keywords
