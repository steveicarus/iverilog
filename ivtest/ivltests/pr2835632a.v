// This checks various constant selects using the indexed select operators
// +: and -: for both big and little endian vectors.

`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  parameter base = -1;

  parameter [base+15:base] p_big = 16'h0123;
  parameter [base:base+15] p_ltl = 16'h3210;
  parameter p_base = 16'h0123;

  reg [base+15:base] big = 16'h0123;
  reg [base:base+15] ltl = 16'h3210;
  reg [base+15:base] big_l;
  reg [base:base+15] ltl_l;

  wire [base+15:base] w_big = 16'h0123;
  wire [base:base+15] w_ltl = 16'h3210;

  reg [3:0] big0, big1, big2, big3, ltl0, ltl1, ltl2, ltl3;
  reg [3:0] big0a, big3a, bigx, bigo, ltl0a, ltl3a, ltlx, ltlo;

  reg pass;

  /*
   * Check a constant +: as a CA R-value.
   */
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  wire [3:0] wcu_big3a = w_big[(base-1)+:4];
`else
  wire [3:0] wcu_big3a = {w_big[(base)+:3],1'bx};
`endif
  wire [3:0] wcu_big3 = w_big[(base)+:4];
  wire [3:0] wcu_big2 = w_big[(base+4)+:4];
  wire [3:0] wcu_big1 = w_big[(base+8)+:4];
  wire [3:0] wcu_big0 = w_big[(base+12)+:4];
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  wire [3:0] wcu_big0a = w_big[(base+13)+:4];
  wire [3:0] wcu_bigx = w_big[(1'bx)+:4];
  wire [3:0] wcu_ltl3a = w_ltl[(base-1)+:4];
`else
  wire [3:0] wcu_big0a = {1'bx,w_big[(base+13)+:3]};
  wire [3:0] wcu_bigx = 4'bxxxx;
  wire [3:0] wcu_ltl3a = {1'bx,w_ltl[(base)+:3]};
`endif
  wire [3:0] wcu_ltl3 = w_ltl[(base)+:4];
  wire [3:0] wcu_ltl2 = w_ltl[(base+4)+:4];
  wire [3:0] wcu_ltl1 = w_ltl[(base+8)+:4];
  wire [3:0] wcu_ltl0 = w_ltl[(base+12)+:4];
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  wire [3:0] wcu_ltl0a = w_ltl[(base+13)+:4];
  wire [3:0] wcu_ltlx = w_ltl[(1'bx)+:4];
`else
  wire [3:0] wcu_ltl0a = {w_ltl[(base+13)+:3],1'bx};
  wire [3:0] wcu_ltlx = 4'bxxxx;
`endif

  /*
   * Check a constant -: as a CA R-value.
   */
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  wire [3:0] wcd_big3a = w_big[(base+2)-:4];
`else
  wire [3:0] wcd_big3a = {w_big[(base+2)-:3],1'bx};
`endif
  wire [3:0] wcd_big3 = w_big[(base+3)-:4];
  wire [3:0] wcd_big2 = w_big[(base+7)-:4];
  wire [3:0] wcd_big1 = w_big[(base+11)-:4];
  wire [3:0] wcd_big0 = w_big[(base+15)-:4];
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  wire [3:0] wcd_big0a = w_big[(base+16)-:4];
  wire [3:0] wcd_bigx = w_big[(1'bx)-:4];
  wire [3:0] wcd_ltl3a = w_ltl[(base+2)-:4];
`else
  wire [3:0] wcd_big0a = {1'bx,w_big[(base+15)-:3]};
  wire [3:0] wcd_bigx = 4'bxxxx;
  wire [3:0] wcd_ltl3a = {1'bx,w_ltl[(base+2)-:3]};
`endif
  wire [3:0] wcd_ltl3 = w_ltl[(base+3)-:4];
  wire [3:0] wcd_ltl2 = w_ltl[(base+7)-:4];
  wire [3:0] wcd_ltl1 = w_ltl[(base+11)-:4];
  wire [3:0] wcd_ltl0 = w_ltl[(base+15)-:4];
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  wire [3:0] wcd_ltl0a = w_ltl[(base+16)-:4];
  wire [3:0] wcd_ltlx = w_ltl[(1'bx)-:4];
`else
  wire [3:0] wcd_ltl0a = {w_ltl[(base+15)-:3],1'bx};
  wire [3:0] wcd_ltlx = 4'bxxxx;
`endif

  /*
   * Check a constant +: as a CA L-value.
   */
  wire [base+15:base] wcu_big_l;
  wire [base:base+15] wcu_ltl_l;

  assign wcu_big_l[(base)+:4] = 4'd3;
  assign wcu_big_l[(base+4)+:4] = 4'd2;
  assign wcu_big_l[(base+8)+:4] = 4'd1;
  assign wcu_big_l[(base+12)+:4] = 4'd0;
  assign wcu_ltl_l[(base)+:4] = 4'd3;
  assign wcu_ltl_l[(base+4)+:4] = 4'd2;
  assign wcu_ltl_l[(base+8)+:4] = 4'd1;
  assign wcu_ltl_l[(base+12)+:4] = 4'd0;

  /*
   * Check a constant -: as a CA L-value.
   */
  wire [base+15:base] wcd_big_l;
  wire [base:base+15] wcd_ltl_l;

  assign wcd_big_l[(base+3)-:4] = 4'd3;
  assign wcd_big_l[(base+7)-:4] = 4'd2;
  assign wcd_big_l[(base+11)-:4] = 4'd1;
  assign wcd_big_l[(base+15)-:4] = 4'd0;
  assign wcd_ltl_l[(base+3)-:4] = 4'd3;
  assign wcd_ltl_l[(base+7)-:4] = 4'd2;
  assign wcd_ltl_l[(base+11)-:4] = 4'd1;
  assign wcd_ltl_l[(base+15)-:4] = 4'd0;

  /*
   * Check a constant +: and -: with a 'bx index as a CA L-value.
   */
  wire [base+15:base] wcu_big_lx;
  wire [base:base+15] wcu_ltl_lx;
  wire [base+15:base] wcd_big_lx;
  wire [base:base+15] wcd_ltl_lx;

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  assign wcu_big_lx[(1'bx)+:4] = 4'hf;
  assign wcu_ltl_lx[(1'bx)+:4] = 4'hf;
  assign wcd_big_lx[(1'bx)-:4] = 4'hf;
  assign wcd_ltl_lx[(1'bx)-:4] = 4'hf;
`endif

  /*
   * Check a constant +: and -: with out of bounds values as a CA L-value.
   */
  wire [base+15:base] wcu_big_lo;
  wire [base:base+15] wcu_ltl_lo;
  wire [base+15:base] wcd_big_lo;
  wire [base:base+15] wcd_ltl_lo;

// For now Icarus does not support before base selects in a CA L-value.
// This test needs to be updated when this is added.
//  assign wcu_big_lo[(base-1)+:4] = 4'b011x;
  assign wcu_big_lo[(base)+:3] = 3'b011;
  assign wcu_big_lo[(base+3)+:10] = 10'b0;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  assign wcu_big_lo[(base+13)+:4] = 4'bx001;
  assign wcu_ltl_lo[(base-1)+:4] = 4'bx001;
`else
  assign wcu_big_lo[(base+13)+:3] = 3'b001;
  assign wcu_ltl_lo[(base)+:3] = 3'b001;
`endif
  assign wcu_ltl_lo[(base+3)+:10] = 10'b0;
  assign wcu_ltl_lo[(base+13)+:3] = 3'b011;
//  assign wcu_ltl_lo[(base+13)+:4] = 4'b011x;

//  assign wcd_big_lo[(base+2)-:4] = 4'b011x;
  assign wcd_big_lo[(base+2)-:3] = 3'b011;
  assign wcd_big_lo[(base+12)-:10] = 10'b0;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
  assign wcd_big_lo[(base+16)-:4] = 4'bx001;
  assign wcd_ltl_lo[(base+2)-:4] = 4'bx001;
`else
  assign wcd_big_lo[(base+15)-:3] = 3'b001;
  assign wcd_ltl_lo[(base+2)-:3] = 3'b001;
`endif
  assign wcd_ltl_lo[(base+12)-:10] = 10'b0;
  assign wcd_ltl_lo[(base+15)-:3] = 3'b011;
//  assign wcd_ltl_lo[(base+16)-:4] = 4'b011x;

  initial begin
    pass = 1'b1;
    #1;

    $displayh("p_big/big: %h, p_ltl/ltl: %h, base: %0d", p_big, p_ltl, base);

    /*
     * Check a constant +: on a parameter.
     */
    $display();

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("p_big[%0d+:4]: ", base-1, p_big[(base-1)+:4],
              ", p_ltl[%0d+:4]: ", base-1, p_ltl[(base-1)+:4]);
`else
    $displayb("p_big[%0d+:4]: ", base-1, {p_big[(base)+:3],1'bx},
              ", p_ltl[%0d+:4]: ", base-1, {1'bx,p_ltl[(base)+:3]});
`endif
    $displayh("p_big[%0d+:4]: ", base, p_big[(base)+:4],
              ", p_ltl[%0d+:4]: ", base, p_ltl[(base)+:4]);
    $displayh("p_big[%0d+:4]: ", base+4, p_big[(base+4)+:4],
              ", p_ltl[%0d+:4]: ", base+4, p_ltl[(base+4)+:4]);
    $displayh("p_big[%0d+:4]: ", base+8, p_big[(base+8)+:4],
              ", p_ltl[%0d+:4]: ", base+8, p_ltl[(base+8)+:4]);
    $displayh("p_big[%0d+:4]: ", base+12, p_big[(base+12)+:4],
              ", p_ltl[%0d+:4]: ", base+12, p_ltl[(base+12)+:4]);
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("p_big[%0d+:4]: ", base+13, p_big[(base+13)+:4],
              ", p_ltl[%0d+:4]: ", base+13, p_ltl[(base+13)+:4]);
    $displayb("p_big[%0d+:4]: ", 1'bx, p_big[(1'bx)+:4],
              ", p_ltl[%0d+:4]: ", 1'bx, p_ltl[(1'bx)+:4]);
`else
    $displayb("p_big[%0d+:4]: ", base+13, {1'bx,p_big[(base+13)+:3]},
              ", p_ltl[%0d+:4]: ", base+13, {p_ltl[(base+13)+:3],1'bx});
    $displayb("p_big[%0d+:4]: ", 1'bx, 4'bxxxx,
              ", p_ltl[%0d+:4]: ", 1'bx, 4'bxxxx);
`endif
    if (p_big[  (base) +: 4] !== 4'd3 || p_big[ (base+4) +: 4] !== 4'd2 ||
        p_big[(base+8) +: 4] !== 4'd1 || p_big[(base+12) +: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        p_big[ (base-1) +: 4] !== 4'b011x ||
        p_big[(base+13) +: 4] !== 4'bx000 ||
        p_big[(1'bx) +: 4] !== 4'bxxxx) begin
`else
        {p_big[   (base) +: 3],1'bx} !== 4'b011x ||
        {1'bx,p_big[(base+13) +: 3]} !== 4'bx000 ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: big endian parameter constant +: indexed select.");
      pass = 1'b0;
    end
    if (p_ltl[  (base) +: 4] !== 4'd3 || p_ltl[ (base+4) +: 4] !== 4'd2 ||
        p_ltl[(base+8) +: 4] !== 4'd1 || p_ltl[(base+12) +: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        p_ltl[ (base-1) +: 4] !== 4'bx001 ||
        p_ltl[(base+13) +: 4] !== 4'b000x ||
        p_ltl[(1'bx) +: 4] !== 4'bxxxx) begin
`else
        {1'bx,p_ltl[   (base) +: 3]} !== 4'bx001 ||
        {p_ltl[(base+13) +: 3],1'bx} !== 4'b000x ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: little endian parameter constant +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant -: on a parameter.
     */
    $display();
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("p_big[%0d-:4]: ", base+2, p_big[(base+2)-:4],
              ", p_ltl[%0d-:4]: ", base+2, p_ltl[(base+2)-:4]);
`else
    $displayb("p_big[%0d-:4]: ", base+2, {p_big[(base+2)-:3],1'bx},
              ", p_ltl[%0d-:4]: ", base+2, {1'bx,p_ltl[(base+2)-:3]});
`endif
    $displayh("p_big[%0d-:4]: ", base+3, p_big[(base+3)-:4],
              ", p_ltl[%0d-:4]: ", base+3, p_ltl[(base+3)-:4]);
    $displayh("p_big[%0d-:4]: ", base+7, p_big[(base+7)-:4],
              ", p_ltl[%0d-:4]: ", base+7, p_ltl[(base+7)-:4]);
    $displayh("p_big[%0d-:4]: ", base+11, p_big[(base+11)-:4],
              ", p_ltl[%0d-:4]: ", base+11, p_ltl[(base+11)-:4]);
    $displayh("p_big[%0d-:4]: ", base+15, p_big[(base+15)-:4],
              ", p_ltl[%0d-:4]: ", base+15, p_ltl[(base+15)-:4]);
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("p_big[%0d-:4]: ", base+16, p_big[(base+16)-:4],
              ", p_ltl[%0d-:4]: ", base+16, p_ltl[(base+16)-:4]);
    $displayb("p_big[%0d-:4]: ", 1'bx, p_big[(1'bx)-:4],
              ", p_ltl[%0d-:4]: ", 1'bx, p_ltl[(1'bx)-:4]);
`else
    $displayb("p_big[%0d-:4]: ", base+16, {1'bx,p_big[(base+15)-:3]},
              ", p_ltl[%0d-:4]: ", base+16, {p_ltl[(base+15)-:3],1'bx});
    $displayb("p_big[%0d-:4]: ", 1'bx, 4'bxxxx,
              ", p_ltl[%0d-:4]: ", 1'bx, 4'bxxxx);
`endif
    if (p_big[ (base+3) -: 4] !== 4'd3 || p_big[ (base+7) -: 4] !== 4'd2 ||
        p_big[(base+11) -: 4] !== 4'd1 || p_big[(base+15) -: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        p_big[ (base+2) -: 4] !== 4'b011x ||
        p_big[(base+16) -: 4] !== 4'bx000 ||
        p_big[(1'bx) -: 4] !== 4'bxxxx) begin
`else
        {p_big[ (base+2) -: 3],1'bx} !== 4'b011x ||
        {1'bx,p_big[(base+15) -: 3]} !== 4'bx000 ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: big endian parameter constant -: indexed select.");
      pass = 1'b0;
    end
    if (p_ltl[ (base+3) -: 4] !== 4'd3 || p_ltl[ (base+7) -: 4] !== 4'd2 ||
        p_ltl[(base+11) -: 4] !== 4'd1 || p_ltl[(base+15) -: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        p_ltl[ (base+2) -: 4] !== 4'bx001 ||
        p_ltl[(base+16) -: 4] !== 4'b000x ||
        p_ltl[(1'bx) -: 4] !== 4'bxxxx) begin
`else
        {1'bx,p_ltl[ (base+2) -: 3]} !== 4'bx001 ||
        {p_ltl[(base+15) -: 3],1'bx} !== 4'b000x ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: little endian parameter constant -: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant +: on a parameter with out a width specification.
     */
    $display();
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("p_base[-1+:4]: ", p_base[-1+:4]);
`else
    $displayb("p_base[-1+:4]: ", {p_base[0+:3],1'bx});
`endif
    $displayh("p_base[0+:4]: ", p_base[0+:4]);
    $displayh("p_base[4+:4]: ", p_base[4+:4]);
    $displayh("p_base[8+:4]: ", p_base[8+:4]);
    $displayh("p_base[12+:4]: ", p_base[12+:4]);
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("p_base[13+:4]: ", p_base[13+:4]);
    $displayb("p_base[x+:4]: ", p_base[(1'bx)+:4]);
`else
    $displayb("p_base[13+:4]: ", {1'bx,p_base[13+:3]});
    $displayb("p_base[x+:4]: ", 4'bxxxx);
`endif
    if (p_base[ 0 +: 4] !== 4'd3 || p_base[ 4 +: 4] !== 4'd2 ||
        p_base[ 8 +: 4] !== 4'd1 || p_base[12 +: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        p_base[-1 +: 4] !== 4'b011x || p_base[13 +: 4] !== 4'bx000 ||
        p_base[1'bx +: 4] !== 4'bxxxx) begin
`else
        {p_base[0 +: 3],1'bx} !== 4'b011x ||
        {1'bx,p_base[13 +: 3]} !== 4'bx000 ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: base parameter constant +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant -: on a parameter with out a width specification.
     */
    $display();
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("p_base[2-:4]: ", p_base[2-:4]);
`else
    $displayb("p_base[2-:4]: ", {p_base[2-:3],1'bx});
`endif
    $displayh("p_base[3-:4]: ", p_base[3-:4]);
    $displayh("p_base[7-:4]: ", p_base[7-:4]);
    $displayh("p_base[11-:4]: ", p_base[11-:4]);
    $displayh("p_base[15-:4]: ", p_base[15-:4]);
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("p_base[16-:4]: ", p_base[16-:4]);
    $displayb("p_base[x-:4]: ", p_base[(1'bx)-:4]);
`else
    $displayb("p_base[16-:4]: ", {1'bx,p_base[15-:3]});
    $displayb("p_base[x-:4]: ", 4'bxxxx);
`endif
    if (p_base[ 3 -: 4] !== 4'd3 || p_base[ 7 -: 4] !== 4'd2 ||
        p_base[11 -: 4] !== 4'd1 || p_base[15 -: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        p_base[ 2 -: 4] !== 4'b011x || p_base[16 -: 4] !== 4'bx000 ||
        p_base[(1'bx) -: 4] !== 4'bxxxx) begin
`else
        {p_base[ 2 -: 3],1'bx} !== 4'b011x ||
        {1'bx,p_base[15 -: 3]} !== 4'bx000 ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: base parameter constant -: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant +: on a register.
     */
    $display();
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("big[%0d+:4]: ", base-1, big[(base-1)+:4],
              ", ltl[%0d+:4]: ", base-1, ltl[(base-1)+:4]);
`else
    $displayb("big[%0d+:4]: ", base-1, {big[(base)+:3],1'bx},
              ", ltl[%0d+:4]: ", base-1, {1'bx,ltl[(base)+:3]});
`endif
    $displayh("big[%0d+:4]: ", base, big[(base)+:4],
              ", ltl[%0d+:4]: ", base, ltl[(base)+:4]);
    $displayh("big[%0d+:4]: ", base+4, big[(base+4)+:4],
               ", ltl[%0d+:4]: ", base+4, ltl[(base+4)+:4]);
    $displayh("big[%0d+:4]: ", base+8, big[(base+8)+:4],
               ", ltl[%0d+:4]: ", base+8, ltl[(base+8)+:4]);
    $displayh("big[%0d+:4]: ",base+12, big[(base+12)+:4],
               ", ltl[%0d+:4]: ",base+12, ltl[(base+12)+:4]);
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("big[%0d+:4]: ",base+13, big[(base+13)+:4],
               ", ltl[%0d+:4]: ",base+13, ltl[(base+13)+:4]);
    $displayb("big[%0d+:4]: ",1'bx, big[(1'bx)+:4],
               ", ltl[%0d+:4]: ",1'bx, ltl[(1'bx)+:4]);
`else
    $displayb("big[%0d+:4]: ",base+13, {1'bx,big[(base+13)+:3]},
               ", ltl[%0d+:4]: ",base+13, {ltl[(base+13)+:3],1'bx});
    $displayb("big[%0d+:4]: ",1'bx, 4'bxxxx,
               ", ltl[%0d+:4]: ",1'bx, 4'bxxxx);
`endif
    if (big[  (base) +: 4] !== 4'd3 || big[ (base+4) +: 4] !== 4'd2 ||
        big[(base+8) +: 4] !== 4'd1 || big[(base+12) +: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        big[ (base-1) +: 4] !== 4'b011x ||
        big[(base+13) +: 4] !== 4'bx000 ||
        big[(1'bx) +: 4] !== 4'bxxxx) begin
`else
        {big[   (base) +: 3],1'bx} !== 4'b011x ||
        {1'bx,big[(base+13) +: 3]} !== 4'bx000 ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: big endian register constant +: indexed select.");
      pass = 1'b0;
    end
    if (ltl[  (base) +: 4] !== 4'd3 || ltl[ (base+4) +: 4] !== 4'd2 ||
        ltl[(base+8) +: 4] !== 4'd1 || ltl[(base+12) +: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        ltl[ (base-1) +: 4] !== 4'bx001 ||
        ltl[(base+13) +: 4] !== 4'b000x ||
        ltl[(1'bx) +: 4] !== 4'bxxxx) begin
`else
        {1'bx,ltl[   (base) +: 3]} !== 4'bx001 ||
        {ltl[(base+13) +: 3],1'bx} !== 4'b000x ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: little endian register constant +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant -: on a register.
     */
    $display();
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("big[%0d-:4]: ", base+2, big[(base+2)-:4],
              ", ltl[%0d-:4]: ", base+2, ltl[(base+2)-:4]);
`else
    $displayb("big[%0d-:4]: ", base+2, {big[(base+2)-:3],1'bx},
              ", ltl[%0d-:4]: ", base+2, {1'bx,ltl[(base+2)-:3]});
`endif
    $displayh("big[%0d-:4]: ", base+3, big[(base+3)-:4],
              ", ltl[%0d-:4]: ", base+3, ltl[(base+3)-:4]);
    $displayh("big[%0d-:4]: ", base+7, big[(base+7)-:4],
              ", ltl[%0d-:4]: ", base+7, ltl[(base+7)-:4]);
    $displayh("big[%0d-:4]: ", base+11, big[(base+11)-:4],
              ", ltl[%0d-:4]: ", base+11, ltl[(base+11)-:4]);
    $displayh("big[%0d-:4]: ", base+15, big[(base+15)-:4],
              ", ltl[%0d-:4]: ", base+15, ltl[(base+15)-:4]);
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("big[%0d-:4]: ", base+16, big[(base+16)-:4],
              ", ltl[%0d-:4]: ", base+16, ltl[(base+16)-:4]);
    $displayb("big[%0d-:4]: ", 1'bx, big[(1'bx)-:4],
              ", ltl[%0d-:4]: ", 1'bx, ltl[(1'bx)-:4]);
`else
    $displayb("big[%0d-:4]: ", base+16, {1'bx,big[(base+15)-:3]},
              ", ltl[%0d-:4]: ", base+16, {ltl[(base+15)-:3],1'bx});
    $displayb("big[%0d-:4]: ", 1'bx, 4'bxxxx,
              ", ltl[%0d-:4]: ", 1'bx, 4'bxxxx);
`endif
    if (big[ (base+3) -: 4] !== 4'd3 || big[ (base+7) -: 4] !== 4'd2 ||
        big[(base+11) -: 4] !== 4'd1 || big[(base+15) -: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        big[ (base+2) -: 4] !== 4'b011x ||
        big[(base+16) -: 4] !== 4'bx000 ||
        big[(1'bx) -: 4] !== 4'bxxxx) begin
`else
        {big[ (base+2) -: 3],1'bx} !== 4'b011x ||
        {1'bx,big[(base+15) -: 3]} !== 4'bx000 ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: big endian register constant -: indexed select.");
      pass = 1'b0;
    end
    if (ltl[ (base+3) -: 4] !== 4'd3 || ltl[ (base+7) -: 4] !== 4'd2 ||
        ltl[(base+11) -: 4] !== 4'd1 || ltl[(base+15) -: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        ltl[ (base+2) -: 4] !== 4'bx001 ||
        ltl[(base+16) -: 4] !== 4'b000x ||
        ltl[(1'bx) -: 4] !== 4'bxxxx) begin
`else
        {1'bx,ltl[ (base+2) -: 3]} !== 4'bx001 ||
        {ltl[(base+15) -: 3],1'bx} !== 4'b000x ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: little endian register constant -: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant +: on a wire.
     */
    $display();
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("w_big[%0d+:4]: ", base-1, w_big[(base-1)+:4],
              ", w_ltl[%0d+:4]: ", base-1, w_ltl[(base-1)+:4]);
`else
    $displayb("w_big[%0d+:4]: ", base-1, {w_big[(base)+:3],1'bx},
              ", w_ltl[%0d+:4]: ", base-1, {1'bx,w_ltl[(base)+:3]});
`endif
    $displayh("w_big[%0d+:4]: ", base, w_big[(base)+:4],
              ", w_ltl[%0d+:4]: ", base, w_ltl[(base)+:4]);
    $displayh("w_big[%0d+:4]: ", base+4, w_big[(base+4)+:4],
              ", w_ltl[%0d+:4]: ", base+4, w_ltl[(base+4)+:4]);
    $displayh("w_big[%0d+:4]: ", base+8, w_big[(base+8)+:4],
              ", w_ltl[%0d+:4]: ", base+8, w_ltl[(base+8)+:4]);
    $displayh("w_big[%0d+:4]: ", base+12, w_big[(base+12)+:4],
              ", w_ltl[%0d+:4]: ", base+12, w_ltl[(base+12)+:4]);
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("w_big[%0d+:4]: ", base+13, w_big[(base+13)+:4],
              ", w_ltl[%0d+:4]: ", base+13, w_ltl[(base+13)+:4]);
    $displayb("w_big[%0d+:4]: ", 1'bx, w_big[(1'bx)+:4],
              ", w_ltl[%0d+:4]: ", 1'bx, w_ltl[(1'bx)+:4]);
`else
    $displayb("w_big[%0d+:4]: ", base+13, {1'bx,w_big[(base+13)+:3]},
              ", w_ltl[%0d+:4]: ", base+13, {w_ltl[(base+13)+:3],1'bx});
    $displayb("w_big[%0d+:4]: ", 1'bx, 4'bxxxx,
              ", w_ltl[%0d+:4]: ", 1'bx, 4'bxxxx);
`endif
    if (w_big[  (base) +: 4] !== 4'd3 || w_big[ (base+4) +: 4] !== 4'd2 ||
        w_big[(base+8) +: 4] !== 4'd1 || w_big[(base+12) +: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        w_big[ (base-1) +: 4] !== 4'b011x ||
        w_big[(base+13) +: 4] !== 4'bx000 ||
        w_big[(1'bx) +: 4] !== 4'bxxxx) begin
`else
        {w_big[   (base) +: 3],1'bx} !== 4'b011x ||
        {1'bx,w_big[(base+13) +: 3]} !== 4'bx000 ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: big endian wire constant +: indexed select.");
      pass = 1'b0;
    end
    if (w_ltl[  (base) +: 4] !== 4'd3 || w_ltl[ (base+4) +: 4] !== 4'd2 ||
        w_ltl[(base+8) +: 4] !== 4'd1 || w_ltl[(base+12) +: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        w_ltl[ (base-1) +: 4] !== 4'bx001 ||
        w_ltl[(base+13) +: 4] !== 4'b000x ||
        w_ltl[(1'bx) +: 4] !== 4'bxxxx) begin
`else
        {1'bx,w_ltl[   (base) +: 3]} !== 4'bx001 ||
        {w_ltl[(base+13) +: 3],1'bx} !== 4'b000x ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: little endian wire constant +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant -: on a wire.
     */
    $display();
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("w_big[%0d-:4]: ", base+2, w_big[(base+2)-:4],
              ", w_ltl[%0d-:4]: ", base+2, w_ltl[(base+2)-:4]);
`else
    $displayb("w_big[%0d-:4]: ", base+2, {w_big[(base+2)-:3],1'bx},
              ", w_ltl[%0d-:4]: ", base+2, {1'bx,w_ltl[(base+2)-:3]});
`endif
    $displayh("w_big[%0d-:4]: ", base+3, w_big[(base+3)-:4],
              ", w_ltl[%0d-:4]: ", base+3, w_ltl[(base+3)-:4]);
    $displayh("w_big[%0d-:4]: ", base+7, w_big[(base+7)-:4],
              ", w_ltl[%0d-:4]: ", base+7, w_ltl[(base+7)-:4]);
    $displayh("w_big[%0d-:4]: ", base+11, w_big[(base+11)-:4],
              ", w_ltl[%0d-:4]: ", base+11, w_ltl[(base+11)-:4]);
    $displayh("w_big[%0d-:4]: ", base+15, w_big[(base+15)-:4],
              ", w_ltl[%0d-:4]: ", base+15, w_ltl[(base+15)-:4]);
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $displayb("w_big[%0d-:4]: ", base+16, w_big[(base+16)-:4],
              ", w_ltl[%0d-:4]: ", base+16, w_ltl[(base+16)-:4]);
    $displayb("w_big[%0d-:4]: ", 1'bx, w_big[(1'bx)-:4],
              ", w_ltl[%0d-:4]: ", 1'bx, w_ltl[(1'bx)-:4]);
`else
    $displayb("w_big[%0d-:4]: ", base+16, {1'bx,w_big[(base+15)-:3]},
              ", w_ltl[%0d-:4]: ", base+16, {w_ltl[(base+15)-:3],1'bx});
    $displayb("w_big[%0d-:4]: ", 1'bx, 4'bxxxx,
              ", w_ltl[%0d-:4]: ", 1'bx, 4'bxxxx);
`endif
    if (w_big[ (base+3) -: 4] !== 4'd3 || w_big[ (base+7) -: 4] !== 4'd2 ||
        w_big[(base+11) -: 4] !== 4'd1 || w_big[(base+15) -: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        w_big[ (base+2) -: 4] !== 4'b011x ||
        w_big[(base+16) -: 4] !== 4'bx000 ||
        w_big[(1'bx) -: 4] !== 4'bxxxx) begin
`else
        {w_big[ (base+2) -: 3],1'bx} !== 4'b011x ||
        {1'bx,w_big[(base+15) -: 3]} !== 4'bx000 ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: big endian wire constant -: indexed select.");
      pass = 1'b0;
    end
    if (w_ltl[ (base+3) -: 4] !== 4'd3 || w_ltl[ (base+7) -: 4] !== 4'd2 ||
        w_ltl[(base+11) -: 4] !== 4'd1 || w_ltl[(base+15) -: 4] !== 4'd0 ||
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
        w_ltl[ (base+2) -: 4] !== 4'bx001 ||
        w_ltl[(base+16) -: 4] !== 4'b000x ||
        w_ltl[(1'bx) -: 4] !== 4'bxxxx) begin
`else
        {1'bx,w_ltl[ (base+2) -: 3]} !== 4'bx001 ||
        {w_ltl[(base+15) -: 3],1'bx} !== 4'b000x ||
        4'bxxxx !== 4'bxxxx) begin
`endif
      $display("FAILED: little endian wire constant -: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant +: on a CA R-value.
     */
    $display();
    $displayb("wcu_big3a: ", wcu_big3a, ", wcu_ltl3a: ", wcu_ltl3a);
    $displayh("wcu_big3: ", wcu_big3, ", wcu_ltl3: ", wcu_ltl3);
    $displayh("wcu_big2: ", wcu_big2, ", wcu_ltl2: ", wcu_ltl2);
    $displayh("wcu_big1: ", wcu_big1, ", wcu_ltl1: ", wcu_ltl1);
    $displayh("wcu_big0: ", wcu_big0, ", wcu_ltl0: ", wcu_ltl0);
    $displayb("wcu_big0a: ", wcu_big0a, ", wcu_ltl0a: ", wcu_ltl0a);
    $displayb("wcu_bigx: ", wcu_bigx, ", wcu_ltlx: ", wcu_ltlx);
    if (wcu_big3 !== 4'd3 || wcu_big2 !== 4'd2 ||
        wcu_big1 !== 4'd1 || wcu_big0 !== 4'd0 ||
        wcu_big3a !== 4'b011x || wcu_big0a !== 4'bx000 ||
        wcu_bigx !== 4'bxxxx) begin
      $display("FAILED: big endian CA R-value constant +: indexed select.");
      pass = 1'b0;
    end
    if (wcu_ltl3 !== 4'd3 || wcu_ltl2 !== 4'd2 ||
        wcu_ltl1 !== 4'd1 || wcu_ltl0 !== 4'd0 ||
        wcu_ltl3a !== 4'bx001 || wcu_ltl0a !== 4'b000x ||
        wcu_ltlx !== 4'bxxxx) begin
      $display("FAILED: little endian CA R-value constant +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant -: on a CA R-value.
     */
    $display();
    $displayb("wcd_big3a: ", wcd_big3a, ", wcd_ltl3a: ", wcd_ltl3a);
    $displayh("wcd_big3: ", wcd_big3, ", wcd_ltl3: ", wcd_ltl3);
    $displayh("wcd_big2: ", wcd_big2, ", wcd_ltl2: ", wcd_ltl2);
    $displayh("wcd_big1: ", wcd_big1, ", wcd_ltl1: ", wcd_ltl1);
    $displayh("wcd_big0: ", wcd_big0, ", wcd_ltl0: ", wcd_ltl0);
    $displayb("wcd_big0a: ", wcd_big0a, ", wcd_ltl0a: ", wcd_ltl0a);
    $displayb("wcd_bigx: ", wcd_bigx, ", wcd_ltlx: ", wcd_ltlx);
    if (wcd_big3 !== 4'd3 || wcd_big2 !== 4'd2 ||
        wcd_big1 !== 4'd1 || wcd_big0 !== 4'd0 ||
        wcd_big3a !== 4'b011x || wcd_big0a !== 4'bx000 ||
        wcd_bigx !== 4'bxxxx) begin
      $display("FAILED: big endian CA R-value constant -: indexed select.");
      pass = 1'b0;
    end
    if (wcd_ltl3 !== 4'd3 || wcd_ltl2 !== 4'd2 ||
        wcd_ltl1 !== 4'd1 || wcd_ltl0 !== 4'd0 ||
        wcd_ltl3a !== 4'bx001 || wcd_ltl0a !== 4'b000x ||
        wcd_ltlx !== 4'bxxxx) begin
      $display("FAILED: little endian CA R-value constant -: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant +: on a simple L-value.
     */
    $display();
    big_l = 16'hxxxx;
    ltl_l = 16'hxxxx;
    big_l[(base)+:4] = 4'd3;
    ltl_l[(base)+:4] = 4'd3;
    big_l[(base+4)+:4] = 4'd2;
    ltl_l[(base+4)+:4] = 4'd2;
    big_l[(base+8)+:4] = 4'd1;
    ltl_l[(base+8)+:4] = 4'd1;
    big_l[(base+12)+:4] = 4'd0;
    ltl_l[(base+12)+:4] = 4'd0;
    $displayh("big_l[simple]: ", big_l, ", ltl_l[simple]: ", ltl_l);
    if (big_l !== big) begin
      $display("FAILED: big endian simple L-value constant +: indexed select.");
      pass = 1'b0;
    end
    if (ltl_l !== ltl) begin
      $display("FAILED: little endian simple L-value constant +: indexed select.");
      pass = 1'b0;
    end

    big_l = 16'hxxxx;
    ltl_l = 16'hxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    big_l[(1'bx)+:4] = 4'd0;
    ltl_l[(1'bx)+:4] = 4'd0;
`endif
    $displayh("big_l[1'bx]: ", big_l, ", ltl_l[1'bx]: ", ltl_l);
    if (big_l !== 16'hxxxx) begin
      $display("FAILED: big endian L-value constant 'bx index +: indexed select.");
      pass = 1'b0;
    end
    if (ltl_l !== 16'hxxxx) begin
      $display("FAILED: little endian L-value constant 'bx index +: indexed select.");
      pass = 1'b0;
    end

    big_l = 16'h0000;
    ltl_l = 16'h0000;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    big_l[(base-1)+:4] = 4'b011x;
    ltl_l[(base-1)+:4] = 4'bx001;
    big_l[(base+13)+:4] = 4'bx001;
    ltl_l[(base+13)+:4] = 4'b011x;
`else
    big_l[(base)+:3] = 3'b011;
    ltl_l[(base)+:3] = 3'b001;
    big_l[(base+13)+:3] = 3'b001;
    ltl_l[(base+13)+:3] = 3'b011;
`endif
    $displayh("big_l[edge]: ", big_l, ", ltl_l[edge]: ", ltl_l);
    if (big_l !== 16'h2003) begin
      $display("FAILED: big endian edge L-value constant +: indexed select.");
      pass = 1'b0;
    end
    if (ltl_l !== 16'h2003) begin
      $display("FAILED: little endian edge L-value constant +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant -: on a simple L-value.
     */
    $display();
    big_l = 16'hxxxx;
    ltl_l = 16'hxxxx;
    big_l[(base+3)-:4] = 4'd3;
    ltl_l[(base+3)-:4] = 4'd3;
    big_l[(base+7)-:4] = 4'd2;
    ltl_l[(base+7)-:4] = 4'd2;
    big_l[(base+11)-:4] = 4'd1;
    ltl_l[(base+11)-:4] = 4'd1;
    big_l[(base+15)-:4] = 4'd0;
    ltl_l[(base+15)-:4] = 4'd0;
    $displayh("big_l: ", big_l, ", ltl_l: ", ltl_l);
    if (big_l !== big) begin
      $display("FAILED: big endian simple L-value constant -: indexed select.");
      pass = 1'b0;
    end
    if (ltl_l !== ltl) begin
      $display("FAILED: little endian simple L-value constant -: indexed select.");
      pass = 1'b0;
    end

    big_l = 16'hxxxx;
    ltl_l = 16'hxxxx;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    big_l[(1'bx)-:4] = 4'd0;
    ltl_l[(1'bx)-:4] = 4'd0;
`endif
    $displayh("big_l[1'bx]: ", big_l, ", ltl_l[1'bx]: ", ltl_l);
    if (big_l !== 16'hxxxx) begin
      $display("FAILED: big endian L-value constant 'bx index -: indexed select.");
      pass = 1'b0;
    end
    if (ltl_l !== 16'hxxxx) begin
      $display("FAILED: little endian L-value constant 'bx index -: indexed select.");
      pass = 1'b0;
    end

    big_l = 16'h0000;
    ltl_l = 16'h0000;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    big_l[(base+2)-:4] = 4'b011x;
    ltl_l[(base+2)-:4] = 4'bx001;
    big_l[(base+16)-:4] = 4'bx001;
    ltl_l[(base+16)-:4] = 4'b011x;
`else
    big_l[(base+2)-:3] = 3'b011;
    ltl_l[(base+2)-:3] = 3'b001;
    big_l[(base+15)-:3] = 3'b001;
    ltl_l[(base+15)-:3] = 3'b011;
`endif
    $displayh("big_l[edge]: ", big_l, ", ltl_l[edge]: ", ltl_l);
    if (big_l !== 16'h2003) begin
      $display("FAILED: big endian edge L-value constant -: indexed select.");
      pass = 1'b0;
    end
    if (ltl_l !== 16'h2003) begin
      $display("FAILED: little endian edge L-value constant -: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant +: on a CA L-value.
     */
    $display();
    $displayh("wcu_big_l: ", wcu_big_l, ", wcu_ltl_l: ", wcu_ltl_l);
    if (wcu_big_l !== big) begin
      $display("FAILED: big endian CA L-value constant +: indexed select.");
      pass = 1'b0;
    end
    if (wcu_ltl_l !== ltl) begin
      $display("FAILED: little endian CA L-value constant +: indexed select.");
      pass = 1'b0;
    end

    $displayh("wcu_big_lx: ", wcu_big_lx, ", wcu_ltl_lx: ", wcu_ltl_lx);
    if (wcu_big_lx !== 16'hzzzz) begin
      $display("FAILED: big endian CA L-value constant 'bx +: indexed select.");
      pass = 1'b0;
    end
    if (wcu_ltl_lx !== 16'hzzzz) begin
      $display("FAILED: little endian CA L-value constant 'bx +: indexed select.");
      pass = 1'b0;
    end

    $displayh("wcu_big_lo: ", wcu_big_lo, ", wcu_ltl_lo: ", wcu_ltl_lo);
    if (wcu_big_lo !== 16'h2003) begin
      $display("FAILED: big endian edge CA L-value constant +: indexed select.");
      pass = 1'b0;
    end
    if (wcu_ltl_lo !== 16'h2003) begin
      $display("FAILED: little endian edge CA L-value constant +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a constant -: on a CA L-value.
     */
    $display();
    $displayh("wcd_big_l: ", wcd_big_l, ", wcd_ltl_l: ", wcd_ltl_l);
    if (wcd_big_l !== big) begin
      $display("FAILED: big endian CA L-value constant -: indexed select.");
      pass = 1'b0;
    end
    if (wcd_ltl_l !== ltl) begin
      $display("FAILED: little endian CA L-value constant -: indexed select.");
      pass = 1'b0;
    end

    $displayh("wcd_big_lx: ", wcd_big_lx, ", wcd_ltl_lx: ", wcd_ltl_lx);
    if (wcd_big_lx !== 16'hzzzz) begin
      $display("FAILED: big endian CA L-value constant 'bx -: indexed select.");
      pass = 1'b0;
    end
    if (wcd_ltl_lx !== 16'hzzzz) begin
      $display("FAILED: little endian CA L-value constant 'bx -: indexed select.");
      pass = 1'b0;
    end

    $displayh("wcd_big_lo: ", wcd_big_lo, ", wcd_ltl_lo: ", wcd_ltl_lo);
    if (wcd_big_lo !== 16'h2003) begin
      $display("FAILED: big endian edge CA L-value constant -: indexed select.");
      pass = 1'b0;
    end
    if (wcd_ltl_lo !== 16'h2003) begin
      $display("FAILED: little endian edge CA L-value constant -: indexed select.");
      pass = 1'b0;
    end

    /*
     * All done.
     */
    if (pass) $display("PASSED");
  end
endmodule
