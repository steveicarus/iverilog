// This checks various variable selects using the indexed select operators
//  +: and -: for both big and little endian vectors.
module top;
  parameter base = -1;

  parameter [base+15:base] p_big = 16'h0123;
  parameter [base:base+15] p_ltl = 16'h3210;

  reg [base+15:base] big = 16'h0123;
  reg [base:base+15] ltl = 16'h3210;
  reg [base+15:base] bigr;
  reg [base:base+15] ltlr;

  wire [base+15:base] w_big = 16'h0123;
  wire [base:base+15] w_ltl = 16'h3210;

  integer a;

  reg [3:0] big0, big1, big2, big3, ltl0, ltl1, ltl2, ltl3;
  reg [3:0] big0a, big3a, bigx, ltl0a, ltl3a, ltlx;

  reg pass;

  /*
   * Check a variable +: as a CA R-value.
   */
  wire [3:0] wvu_big = w_big[a+:4];
  wire [3:0] wvu_ltl = w_ltl[a+:4];

  /*
   * Check a variable -: as a CA R-value.
   */
  wire [3:0] wvd_big = w_big[a-:4];
  wire [3:0] wvd_ltl = w_ltl[a-:4];

  initial begin
    pass = 1'b1;
    #1;

    $displayh("p_big/big: %h, p_ltl/ltl: %h, base: %0d", p_big, p_ltl, base);

    /*
     * Check a variable +: on a parameter.
     */
    $display();
    a = base-1;
    big3a = p_big[a+:4];
    ltl3a = p_ltl[a+:4];
    $displayb("a==%0d; p_big[a+:4]: ", a, p_big[a+:4],
              ", p_ltl[a+:4]: ", p_ltl[a+:4]);
    a = base;
    big3 = p_big[a+:4];
    ltl3 = p_ltl[a+:4];
    $displayh("a==%0d; p_big[a+:4]: ", a, p_big[a+:4],
              ", p_ltl[a+:4]: ", p_ltl[a+:4]);
    a = base+4;
    big2 = p_big[a+:4];
    ltl2 = p_ltl[a+:4];
    $displayh("a==%0d; p_big[a+:4]: ", a, p_big[a+:4],
              ", p_ltl[a+:4]: ", p_ltl[a+:4]);
    a = base+8;
    big1 = p_big[a+:4];
    ltl1 = p_ltl[a+:4];
    $displayh("a==%0d; p_big[a+:4]: ", a, p_big[a+:4],
              ", p_ltl[a+:4]: ", p_ltl[a+:4]);
    a = base+12;
    big0 = p_big[a+:4];
    ltl0 = p_ltl[a+:4];
    $displayh("a==%0d; p_big[a+:4]: ", a, p_big[a+:4],
              ", p_ltl[a+:4]: ", p_ltl[a+:4]);
    a = base+13;
    big0a = p_big[a+:4];
    ltl0a = p_ltl[a+:4];
    $displayb("a==%0d; p_big[a+:4]: ", a, p_big[a+:4],
              ", p_ltl[a+:4]: ", p_ltl[a+:4]);
    a = 1'bx;
    bigx = p_big[a+:4];
    ltlx = p_ltl[a+:4];
    $displayb("a==%0d; p_big[a+:4]: ", a, p_big[a+:4],
              ", p_ltl[a+:4]: ", p_ltl[a+:4]);
    if (big3 !== 4'd3 || big2 !== 4'd2 || big1 !== 4'd1 || big0 !== 4'd0 ||
        big3a !== 4'b011x || big0a !== 4'bx000 || bigx !== 4'bxxxx) begin
      $display("FAILED: big endian parameter variable +: indexed select.");
      pass = 1'b0;
    end
    if (ltl3 !== 4'd3 || ltl2 !== 4'd2 || ltl1 !== 4'd1 || ltl0 !== 4'd0 ||
        ltl3a !== 4'bx001 || ltl0a !== 4'b000x || ltlx !== 4'bxxxx) begin
      $display("FAILED: little endian parameter variable +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a variable -: on a parameter.
     */
    $display();
    a = base+2;
    big3a = p_big[a-:4];
    ltl3a = p_ltl[a-:4];
    $displayb("a== %0d; p_big[a-:4]: ", a, p_big[a-:4],
              ", p_ltl[a-:4]: ", p_ltl[a-:4]);
    a = base+3;
    big3 = p_big[a-:4];
    ltl3 = p_ltl[a-:4];
    $displayh("a== %0d; p_big[a-:4]: ", a, p_big[a-:4],
              ", p_ltl[a-:4]: ", p_ltl[a-:4]);
    a = base+7;
    big2 = p_big[a-:4];
    ltl2 = p_ltl[a-:4];
    $displayh("a== %0d; p_big[a-:4]: ", a, p_big[a-:4],
              ", p_ltl[a-:4]: ", p_ltl[a-:4]);
    a = base+11;
    big1 = p_big[a-:4];
    ltl1 = p_ltl[a-:4];
    $displayh("a== %0d; p_big[a-:4]: ", a, p_big[a-:4],
              ", p_ltl[a-:4]: ", p_ltl[a-:4]);
    a = base+15;
    big0 = p_big[a-:4];
    ltl0 = p_ltl[a-:4];
    $displayh("a== %0d; p_big[a-:4]: ", a, p_big[a-:4],
              ", p_ltl[a-:4]: ", p_ltl[a-:4]);
    a = base+16;
    big0a = p_big[a-:4];
    ltl0a = p_ltl[a-:4];
    $displayb("a== %0d; p_big[a-:4]: ", a, p_big[a-:4],
              ", p_ltl[a-:4]: ", p_ltl[a-:4]);
    a = 1'bx;
    bigx = p_big[a-:4];
    ltlx = p_ltl[a-:4];
    $displayb("a== %0d; p_big[a-:4]: ", a, p_big[a-:4],
              ", p_ltl[a-:4]: ", p_ltl[a-:4]);
    if (big3 !== 4'd3 || big2 !== 4'd2 || big1 !== 4'd1 || big0 !== 4'd0 ||
        big3a !== 4'b011x || big0a !== 4'bx000 || bigx !== 4'bxxxx) begin
      $display("FAILED: big endian parameter variable -: indexed select.");
      pass = 1'b0;
    end
    if (ltl3 !== 4'd3 || ltl2 !== 4'd2 || ltl1 !== 4'd1 || ltl0 !== 4'd0 ||
        ltl3a !== 4'bx001 || ltl0a !== 4'b000x || ltlx !== 4'bxxxx) begin
      $display("FAILED: little endian parameter variable -: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a variable +: on a register.
     */
    $display();
    a = base-1;
    big3a = big[a+:4];
    ltl3a = ltl[a+:4];
    $displayb("a==%0d; big[a+:4]: ", a, big[a+:4], ", ltl[a+:4]: ", ltl[a+:4]);
    a = base;
    big3 = big[a+:4];
    ltl3 = ltl[a+:4];
    $displayh("a==%0d; big[a+:4]: ", a, big[a+:4], ", ltl[a+:4]: ", ltl[a+:4]);
    a = base+4;
    big2 = big[a+:4];
    ltl2 = ltl[a+:4];
    $displayh("a==%0d; big[a+:4]: ", a, big[a+:4], ", ltl[a+:4]: ", ltl[a+:4]);
    a = base+8;
    big1 = big[a+:4];
    ltl1 = ltl[a+:4];
    $displayh("a==%0d; big[a+:4]: ", a, big[a+:4], ", ltl[a+:4]: ", ltl[a+:4]);
    a = base+12;
    big0 = big[a+:4];
    ltl0 = ltl[a+:4];
    $displayh("a==%0d; big[a+:4]: ", a, big[a+:4], ", ltl[a+:4]: ", ltl[a+:4]);
    a = base+13;
    big0a = big[a+:4];
    ltl0a = ltl[a+:4];
    $displayb("a==%0d; big[a+:4]: ", a, big[a+:4], ", ltl[a+:4]: ", ltl[a+:4]);
    a = 1'bx;
    bigx = big[a+:4];
    ltlx = ltl[a+:4];
    $displayb("a==%0d; big[a+:4]: ", a, big[a+:4], ", ltl[a+:4]: ", ltl[a+:4]);
    if (big3 !== 4'd3 || big2 !== 4'd2 || big1 !== 4'd1 || big0 !== 4'd0 ||
        big3a !== 4'b011x || big0a !== 4'bx000 || bigx !== 4'bxxxx) begin
      $display("FAILED: big endian register variable +: indexed select.");
      pass = 1'b0;
    end
    if (ltl3 !== 4'd3 || ltl2 !== 4'd2 || ltl1 !== 4'd1 || ltl0 !== 4'd0 ||
        ltl3a !== 4'bx001 || ltl0a !== 4'b000x || ltlx !== 4'bxxxx) begin
      $display("FAILED: little endian register variable +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a variable -: on a register.
     */
    $display();
    a = base+2;
    big3a = big[a-:4];
    ltl3a = ltl[a-:4];
    $displayb("a== %0d; big[a-:4]: ", a, big[a-:4], ", ltl[a-:4]: ", ltl[a-:4]);
    a = base+3;
    big3 = big[a-:4];
    ltl3 = ltl[a-:4];
    $displayh("a== %0d; big[a-:4]: ", a, big[a-:4], ", ltl[a-:4]: ", ltl[a-:4]);
    a = base+7;
    big2 = big[a-:4];
    ltl2 = ltl[a-:4];
    $displayh("a== %0d; big[a-:4]: ", a, big[a-:4], ", ltl[a-:4]: ", ltl[a-:4]);
    a = base+11;
    big1 = big[a-:4];
    ltl1 = ltl[a-:4];
    $displayh("a== %0d; big[a-:4]: ", a, big[a-:4], ", ltl[a-:4]: ", ltl[a-:4]);
    a = base+15;
    big0 = big[a-:4];
    ltl0 = ltl[a-:4];
    $displayh("a== %0d; big[a-:4]: ", a, big[a-:4], ", ltl[a-:4]: ", ltl[a-:4]);
    a = base+16;
    big0a = big[a-:4];
    ltl0a = ltl[a-:4];
    $displayb("a== %0d; big[a-:4]: ", a, big[a-:4], ", ltl[a-:4]: ", ltl[a-:4]);
    a = 1'bx;
    bigx = big[a-:4];
    ltlx = ltl[a-:4];
    $displayb("a== %0d; big[a-:4]: ", a, big[a-:4], ", ltl[a-:4]: ", ltl[a-:4]);
    if (big3 !== 4'd3 || big2 !== 4'd2 || big1 !== 4'd1 || big0 !== 4'd0 ||
        big3a !== 4'b011x || big0a !== 4'bx000 || bigx !== 4'bxxxx) begin
      $display("FAILED: big endian register variable -: indexed select.");
      pass = 1'b0;
    end
    if (ltl3 !== 4'd3 || ltl2 !== 4'd2 || ltl1 !== 4'd1 || ltl0 !== 4'd0 ||
        ltl3a !== 4'bx001 || ltl0a !== 4'b000x || ltlx !== 4'bxxxx) begin
      $display("FAILED: little endian register variable -: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a variable +: on a wire.
     */
    $display();
    a = base-1;
    big3a = w_big[a+:4];
    ltl3a = w_ltl[a+:4];
    $displayb("a==%0d; w_big[a+:4]: ", a, w_big[a+:4],
              ", w_ltl[a+:4]: ", w_ltl[a+:4]);
    a = base;
    big3 = w_big[a+:4];
    ltl3 = w_ltl[a+:4];
    $displayh("a==%0d; w_big[a+:4]: ", a, w_big[a+:4],
              ", w_ltl[a+:4]: ", w_ltl[a+:4]);
    a = base+4;
    big2 = w_big[a+:4];
    ltl2 = w_ltl[a+:4];
    $displayh("a==%0d; w_big[a+:4]: ", a, w_big[a+:4],
              ", w_ltl[a+:4]: ", w_ltl[a+:4]);
    a = base+8;
    big1 = w_big[a+:4];
    ltl1 = w_ltl[a+:4];
    $displayh("a==%0d; w_big[a+:4]: ", a, w_big[a+:4],
              ", w_ltl[a+:4]: ", w_ltl[a+:4]);
    a = base+12;
    big0 = w_big[a+:4];
    ltl0 = w_ltl[a+:4];
    $displayh("a==%0d; w_big[a+:4]: ", a, w_big[a+:4],
              ", w_ltl[a+:4]: ", w_ltl[a+:4]);
    a = base+13;
    big0a = w_big[a+:4];
    ltl0a = w_ltl[a+:4];
    $displayb("a==%0d; w_big[a+:4]: ", a, w_big[a+:4],
              ", w_ltl[a+:4]: ", w_ltl[a+:4]);
    a = 1'bx;
    bigx = w_big[a+:4];
    ltlx = w_ltl[a+:4];
    $displayb("a==%0d; w_big[a+:4]: ", a, w_big[a+:4],
              ", w_ltl[a+:4]: ", w_ltl[a+:4]);
    if (big3 !== 4'd3 || big2 !== 4'd2 || big1 !== 4'd1 || big0 !== 4'd0 ||
        big3a !== 4'b011x || big0a !== 4'bx000 || bigx !== 4'bxxxx) begin
      $display("FAILED: big endian wire variable +: indexed select.");
      pass = 1'b0;
    end
    if (ltl3 !== 4'd3 || ltl2 !== 4'd2 || ltl1 !== 4'd1 || ltl0 !== 4'd0 ||
        ltl3a !== 4'bx001 || ltl0a !== 4'b000x || ltlx !== 4'bxxxx) begin
      $display("FAILED: little endian wire variable +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a variable -: on a wire.
     */
    $display();
    a = base+2;
    big3 = w_big[a-:4];
    ltl3 = w_ltl[a-:4];
    $displayb("a== %0d; w_big[a-:4]: ", a, w_big[a-:4],
              ", w_ltl[a-:4]: ", w_ltl[a-:4]);
    a = base+3;
    big3 = w_big[a-:4];
    ltl3 = w_ltl[a-:4];
    $displayh("a== %0d; w_big[a-:4]: ", a, w_big[a-:4],
              ", w_ltl[a-:4]: ", w_ltl[a-:4]);
    a = base+7;
    big2 = w_big[a-:4];
    ltl2 = w_ltl[a-:4];
    $displayh("a== %0d; w_big[a-:4]: ", a, w_big[a-:4],
              ", w_ltl[a-:4]: ", w_ltl[a-:4]);
    a = base+11;
    big1 = w_big[a-:4];
    ltl1 = w_ltl[a-:4];
    $displayh("a== %0d; w_big[a-:4]: ", a, w_big[a-:4],
              ", w_ltl[a-:4]: ", w_ltl[a-:4]);
    a = base+15;
    big0 = w_big[a-:4];
    ltl0 = w_ltl[a-:4];
    $displayh("a== %0d; w_big[a-:4]: ", a, w_big[a-:4],
              ", w_ltl[a-:4]: ", w_ltl[a-:4]);
    a = base+16;
    big0a = w_big[a-:4];
    ltl0a = w_ltl[a-:4];
    $displayb("a== %0d; w_big[a-:4]: ", a, w_big[a-:4],
              ", w_ltl[a-:4]: ", w_ltl[a-:4]);
    a = 1'bx;
    bigx = w_big[a-:4];
    ltlx = w_ltl[a-:4];
    $displayb("a== %0d; w_big[a-:4]: ", a, w_big[a-:4],
              ", w_ltl[a-:4]: ", w_ltl[a-:4]);
    if (big3 !== 4'd3 || big2 !== 4'd2 || big1 !== 4'd1 || big0 !== 4'd0 ||
        big3a !== 4'b011x || big0a !== 4'bx000 || bigx !== 4'bxxxx) begin
      $display("FAILED: big endian wire variable -: indexed select.");
      pass = 1'b0;
    end
    if (ltl3 !== 4'd3 || ltl2 !== 4'd2 || ltl1 !== 4'd1 || ltl0 !== 4'd0 ||
        ltl3a !== 4'bx001 || ltl0a !== 4'b000x || ltlx !== 4'bxxxx) begin
      $display("FAILED: little endian wire variable -: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a variable +: on a CA R-value.
     */
    $display();
    a = base-1;
    #1;
    big3a = wvu_big;
    ltl3a = wvu_ltl;
    $displayb("a==%0d; wvu_big: ", a, wvu_big, ", wvu_ltl: ", wvu_ltl);
    a = base;
    #1;
    big3 = wvu_big;
    ltl3 = wvu_ltl;
    $displayh("a==%0d; wvu_big: ", a, wvu_big, ", wvu_ltl: ", wvu_ltl);
    a = base+4;
    #1;
    big2 = wvu_big;
    ltl2 = wvu_ltl;
    $displayh("a==%0d; wvu_big: ", a, wvu_big, ", wvu_ltl: ", wvu_ltl);
    a = base+8;
    #1;
    big1 = wvu_big;
    ltl1 = wvu_ltl;
    $displayh("a==%0d; wvu_big: ", a, wvu_big, ", wvu_ltl: ", wvu_ltl);
    a = base+12;
    #1;
    big0 = wvu_big;
    ltl0 = wvu_ltl;
    $displayh("a==%0d; wvu_big: ", a, wvu_big, ", wvu_ltl: ", wvu_ltl);
    a = base+13;
    #1;
    big0a = wvu_big;
    ltl0a = wvu_ltl;
    $displayb("a==%0d; wvu_big: ", a, wvu_big, ", wvu_ltl: ", wvu_ltl);
    a = 1'bx;
    #1;
    bigx = wvu_big;
    ltlx = wvu_ltl;
    $displayb("a==%0d; wvu_big: ", a, wvu_big, ", wvu_ltl: ", wvu_ltl);
    if (big3 !== 4'd3 || big2 !== 4'd2 || big1 !== 4'd1 || big0 !== 4'd0 ||
        big3a !== 4'b011x || big0a !== 4'bx000 || bigx !== 4'bxxxx) begin
      $display("FAILED: big endian CA R-value variable +: indexed select.");
      pass = 1'b0;
    end
    if (ltl3 !== 4'd3 || ltl2 !== 4'd2 || ltl1 !== 4'd1 || ltl0 !== 4'd0 ||
        ltl3a !== 4'bx001 || ltl0a !== 4'b000x || ltlx !== 4'bxxxx) begin
      $display("FAILED: little endian CA R-value variable +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a variable -: on a CA R-value.
     */
    $display();
    a = base+2;
    #1;
    big3a = wvd_big;
    ltl3a = wvd_ltl;
    $displayb("a==%0d; wvd_big: ", a, wvd_big, ", wvd_ltl: ", wvd_ltl);
    a = base+3;
    #1;
    big3 = wvd_big;
    ltl3 = wvd_ltl;
    $displayh("a==%0d; wvd_big: ", a, wvd_big, ", wvd_ltl: ", wvd_ltl);
    a = base+7;
    #1;
    big2 = wvd_big;
    ltl2 = wvd_ltl;
    $displayh("a==%0d; wvd_big: ", a, wvd_big, ", wvd_ltl: ", wvd_ltl);
    a = base+11;
    #1;
    big1 = wvd_big;
    ltl1 = wvd_ltl;
    $displayh("a==%0d; wvd_big: ", a, wvd_big, ", wvd_ltl: ", wvd_ltl);
    a = base+15;
    #1;
    big0 = wvd_big;
    ltl0 = wvd_ltl;
    $displayh("a==%0d; wvd_big: ", a, wvd_big, ", wvd_ltl: ", wvd_ltl);
    a = base+16;
    #1;
    big0a = wvd_big;
    ltl0a = wvd_ltl;
    $displayb("a==%0d; wvd_big: ", a, wvd_big, ", wvd_ltl: ", wvd_ltl);
    a = 1'bx;
    #1;
    bigx = wvd_big;
    ltlx = wvd_ltl;
    $displayb("a==%0d; wvd_big: ", a, wvd_big, ", wvd_ltl: ", wvd_ltl);
    if (big3 !== 4'd3 || big2 !== 4'd2 || big1 !== 4'd1 || big0 !== 4'd0 ||
        big3a !== 4'b011x || big0a !== 4'bx000 || bigx !== 4'bxxxx) begin
      $display("FAILED: big endian CA R-value variable -: indexed select.");
      pass = 1'b0;
    end
    if (ltl3 !== 4'd3 || ltl2 !== 4'd2 || ltl1 !== 4'd1 || ltl0 !== 4'd0 ||
        ltl3a !== 4'bx001 || ltl0a !== 4'b000x || ltlx !== 4'bxxxx) begin
      $display("FAILED: little endian CA R-value variable -: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a variable +: on a simple L-value.
     */
    $display();
    bigr = 16'hxxxx;
    ltlr = 16'hxxxx;
    a = base;
    bigr[(a)+:4] = 4'd3;
    ltlr[(a)+:4] = 4'd3;
    a = base+4;
    bigr[(a)+:4] = 4'd2;
    ltlr[(a)+:4] = 4'd2;
    a = base+8;
    bigr[(a)+:4] = 4'd1;
    ltlr[(a)+:4] = 4'd1;
    a = base+12;
    bigr[(a)+:4] = 4'd0;
    ltlr[(a)+:4] = 4'd0;
    $displayh("bigr[a+:4]: ", bigr, ", ltlr[a+:4]: ", ltlr);
    if (bigr !== big) begin
      $display("FAILED: big endian variable L-value +: indexed select.");
      pass = 1'b0;
    end
    if (ltlr !== ltl) begin
      $display("FAILED: little endian variable L-value +: indexed select.");
      pass = 1'b0;
    end

    bigr = 16'h0000;
    ltlr = 16'h0000;
    a = 'bx;
    bigr[(a)+:4] = 4'hf;
    ltlr[(a)+:4] = 4'hf;
    $displayh("bigr[a+='bx :4]: ", bigr, ", ltlr[a='bx +:4]: ", ltlr);
    if (bigr !== 16'h0000) begin
      $display("FAILED: big endian variable ('bx) L-value +: indexed select.");
      pass = 1'b0;
    end
    if (ltlr !== 16'h0000) begin
      $display("FAILED: little endian variable ('bx) L-value +: indexed select.");
      pass = 1'b0;
    end

    bigr = 16'h0000;
    ltlr = 16'h0000;
    a = base-1;
    bigr[(a)+:4] = 4'b011x;
    ltlr[(a)+:4] = 4'bx001;
    a = base+13;
    bigr[(a)+:4] = 4'bx001;
    ltlr[(a)+:4] = 4'b011x;
    $displayh("bigr[a=edge +:4]: ", bigr, ", ltlr[a=edge +:4]: ", ltlr);
    if (bigr !== 16'h2003) begin
      $display("FAILED: big endian edge L-value constant +: indexed select.");
      pass = 1'b0;
    end
    if (ltlr !== 16'h2003) begin
      $display("FAILED: little endian edge L-value constant +: indexed select.");
      pass = 1'b0;
    end

    /*
     * Check a variable -: on a simple L-value.
     */
    $display();
    bigr = 16'hxxxx;
    ltlr = 16'hxxxx;
    a = base+3;
    bigr[(a)-:4] = 4'd3;
    ltlr[(a)-:4] = 4'd3;
    a = base+7;
    bigr[(a)-:4] = 4'd2;
    ltlr[(a)-:4] = 4'd2;
    a = base+11;
    bigr[(a)-:4] = 4'd1;
    ltlr[(a)-:4] = 4'd1;
    a = base+15;
    bigr[(a)-:4] = 4'd0;
    ltlr[(a)-:4] = 4'd0;
    $displayh("bigr[a-:4]: ", bigr, ", ltlr[a-:4]: ", ltlr);
    if (bigr !== big) begin
      $display("FAILED: big endian variable L-value -: indexed select.");
      pass = 1'b0;
    end
    if (ltlr !== ltl) begin
      $display("FAILED: little endian variable L-value -: indexed select.");
      pass = 1'b0;
    end

    bigr = 16'h0000;
    ltlr = 16'h0000;
    a = 'bx;
    bigr[(a)-:4] = 4'hf;
    ltlr[(a)-:4] = 4'hf;
    $displayh("bigr[a='bx -:4]: ", bigr, ", ltlr[a='bx -:4]: ", ltlr);
    if (bigr !== 16'h0000) begin
      $display("FAILED: big endian variable ('bx) L-value -: indexed select.");
      pass = 1'b0;
    end
    if (ltlr !== 16'h0000) begin
      $display("FAILED: little endian variable ('bx) L-value -: indexed select.");
      pass = 1'b0;
    end

    bigr = 16'h0000;
    ltlr = 16'h0000;
    a = base+2;
    bigr[(a)-:4] = 4'b011x;
    ltlr[(a)-:4] = 4'bx001;
    a = base+16;
    bigr[(a)-:4] = 4'bx001;
    ltlr[(a)-:4] = 4'b011x;
    $displayh("bigr[a=edge -:4]: ", bigr, ", ltlr[a=edge -:4]: ", ltlr);
    if (bigr !== 16'h2003) begin
      $display("FAILED: big endian edge L-value constant -: indexed select.");
      pass = 1'b0;
    end
    if (ltlr !== 16'h2003) begin
      $display("FAILED: little endian edge L-value constant -: indexed select.");
      pass = 1'b0;
    end

    /*
     * All done.
     */
    if (pass) $display("PASSED");
  end
endmodule
