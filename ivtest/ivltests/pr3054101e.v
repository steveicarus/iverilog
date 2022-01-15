// Check the various variable indexed down selects (MSB > LSB).
module top;
  parameter [4:1] ap = 4'h8;
  parameter [4:1] bp = 4'h7;
  parameter [0:-3] cp = 4'h8;
  parameter [0:-3] dp = 4'h7;

  reg passed;

  wire [4:1] a = 4'h8;
  wire [4:1] b = 4'h7;
  wire [0:0] s0 = 1;
  wire [1:0] s1 = 1;
  wire [2:0] s2 = 1;
  reg [4:1] ar = 4'h8;
  reg [4:1] br = 4'h7;

  wire [0:-3] c = 4'h8;
  wire [0:-3] d = 4'h7;
  wire [0:0] s3 = 1;
  wire [1:0] s4 = 1;
  reg [0:-3] cr = 4'h8;
  reg [0:-3] dr = 4'h7;

  wire [1:0] res_a0 = a[s0-:2];
  wire [1:0] res_b0 = b[s0-:2];
  wire [1:0] res_a1 = a[s1-:2];
  wire [1:0] res_b1 = b[s1-:2];
  wire [1:0] res_a2 = a[s2-:2];
  wire [1:0] res_b2 = b[s2-:2];

  wire [1:0] res_c3 = c[s3-:2];
  wire [1:0] res_d3 = d[s3-:2];
  wire [1:0] res_c4 = c[s4-:2];
  wire [1:0] res_d4 = d[s4-:2];

  reg [4:1] res_ab;
  reg [0:-3] res_cd;

  initial begin
    #1;
    passed = 1'b1;

    // Check procedural R-value variable index down selects of a net.

    $display("a[s0-:2]: %b", a[s1-:2]);
    if (a[s0-:2] !== 2'b0x) begin
      $display("Failed a[s0-:2], expected 2'b0x, got %b", a[s0-:2]);
      passed = 1'b0;
    end

    $display("b[s0-:2]: %b", b[s1-:2]);
    if (b[s0-:2] !== 2'b1x) begin
      $display("Failed b[s0-:2], expected 2'b1x, got %b", b[s0-:2]);
      passed = 1'b0;
    end

    $display("a[s1-:2]: %b", a[s1-:2]);
    if (a[s1-:2] !== 2'b0x) begin
      $display("Failed a[s1-:2], expected 2'b0x, got %b", a[s1-:2]);
      passed = 1'b0;
    end

    $display("b[s1-:2]: %b", b[s1-:2]);
    if (b[s1-:2] !== 2'b1x) begin
      $display("Failed b[s1-:2], expected 2'b1x, got %b", b[s1-:2]);
      passed = 1'b0;
    end

    $display("a[s2-:2]: %b", a[s2-:2]);
    if (a[s2-:2] !== 2'b0x) begin
      $display("Failed a[s2-:2], expected 2'b0x, got %b", a[s2-:2]);
      passed = 1'b0;
    end

    $display("b[s2-:2]: %b", b[s2-:2]);
    if (b[s2-:2] !== 2'b1x) begin
      $display("Failed b[s2-:2], expected 2'b1x, got %b", b[s2-:2]);
      passed = 1'b0;
    end

    $display("c[s3-:2]: %b", c[s3-:2]);
    if (c[s3-:2] !== 2'bx1) begin
      $display("Failed c[s3-:2], expected 2'bx1, got %b", c[s3-:2]);
      passed = 1'b0;
    end

    $display("d[s3-:2]: %b", d[s3-:2]);
    if (d[s3-:2] !== 2'bx0) begin
      $display("Failed d[s3-:2], expected 2'bx0, got %b", d[s3-:2]);
      passed = 1'b0;
    end

    $display("c[s4-:2]: %b", c[s4-:2]);
    if (c[s4-:2] !== 2'bx1) begin
      $display("Failed c[s4-:2], expected 2'bx1, got %b", c[s4-:2]);
      passed = 1'b0;
    end

    $display("d[s4-:2]: %b", d[s4-:2]);
    if (d[s4-:2] !== 2'bx0) begin
      $display("Failed d[s4-:2], expected 2'bx0, got %b", d[s4-:2]);
      passed = 1'b0;
    end

    // Check procedural R-value variable index down selects of a parameter.

    $display("ap[s0-:2]: %b", ap[s0-:2]);
    if (ap[s0-:2] !== 2'b0x) begin
      $display("Failed ap[s0-:2], expected 2'b0x, got %b", ap[s0-:2]);
      passed = 1'b0;
    end

    $display("bp[s0-:2]: %b", bp[s0-:2]);
    if (bp[s0-:2] !== 2'b1x) begin
      $display("Failed bp[s0-:2], expected 2'b1x, got %b", bp[s0-:2]);
      passed = 1'b0;
    end

    $display("ap[s1-:2]: %b", ap[s1-:2]);
    if (ap[s1-:2] !== 2'b0x) begin
      $display("Failed ap[s1-:2], expected 2'b0x, got %b", ap[s1-:2]);
      passed = 1'b0;
    end

    $display("bp[s1-:2]: %b", bp[s1-:2]);
    if (bp[s1-:2] !== 2'b1x) begin
      $display("Failed bp[s1-:2], expected 2'b1x, got %b", bp[s1-:2]);
      passed = 1'b0;
    end

    $display("ap[s2-:2]: %b", ap[s2-:2]);
    if (ap[s2-:2] !== 2'b0x) begin
      $display("Failed ap[s2-:2], expected 2'b0x, got %b", ap[s2-:2]);
      passed = 1'b0;
    end

    $display("bp[s2-:2]: %b", bp[s2-:2]);
    if (bp[s2-:2] !== 2'b1x) begin
      $display("Failed bp[s2-:2], expected 2'b1x, got %b", bp[s2-:2]);
      passed = 1'b0;
    end

    $display("cp[s3-:2]: %b", cp[s3-:2]);
    if (cp[s3-:2] !== 2'bx1) begin
      $display("Failed cp[s3-:2], expected 2'bx1, got %b", cp[s3-:2]);
      passed = 1'b0;
    end

    $display("dp[s3-:2]: %b", dp[s3-:2]);
    if (dp[s3-:2] !== 2'bx0) begin
      $display("Failed dp[s3-:2], expected 2'bx0, got %b", dp[s3-:2]);
      passed = 1'b0;
    end

    $display("cp[s4-:2]: %b", cp[s4-:2]);
    if (cp[s4-:2] !== 2'bx1) begin
      $display("Failed cp[s4-:2], expected 2'bx1, got %b", cp[s4-:2]);
      passed = 1'b0;
    end

    $display("dp[s4-:2]: %b", dp[s4-:2]);
    if (dp[s4-:2] !== 2'bx0) begin
      $display("Failed dp[s4-:2], expected 2'bx0, got %b", dp[s4-:2]);
      passed = 1'b0;
    end

    // Check procedural R-value variable index down selects of a reg.

    $display("ar[s0-:2]: %b", ar[s0-:2]);
    if (ar[s0-:2] !== 2'b0x) begin
      $display("Failed ar[s0-:2], expected 2'b0x, got %b", ar[s0-:2]);
      passed = 1'b0;
    end

    $display("br[s0-:2]: %b", br[s0-:2]);
    if (br[s0-:2] !== 2'b1x) begin
      $display("Failed br[s0-:2], expected 2'b1x, got %b", br[s0-:2]);
      passed = 1'b0;
    end

    $display("ar[s1-:2]: %b", ar[s1-:2]);
    if (ar[s1-:2] !== 2'b0x) begin
      $display("Failed ar[s1-:2], expected 2'b0x, got %b", ar[s1-:2]);
      passed = 1'b0;
    end

    $display("br[s1-:2]: %b", br[s1-:2]);
    if (br[s1-:2] !== 2'b1x) begin
      $display("Failed br[s1-:2], expected 2'b1x, got %b", br[s1-:2]);
      passed = 1'b0;
    end

    $display("ar[s2-:2]: %b", ar[s2-:2]);
    if (ar[s2-:2] !== 2'b0x) begin
      $display("Failed ar[s2-:2], expected 2'b0x, got %b", ar[s2-:2]);
      passed = 1'b0;
    end

    $display("br[s2-:2]: %b", br[s2-:2]);
    if (br[s2-:2] !== 2'b1x) begin
      $display("Failed br[s2-:2], expected 2'b1x, got %b", br[s2-:2]);
      passed = 1'b0;
    end

    $display("cr[s3-:2]: %b", cr[s3-:2]);
    if (cr[s3-:2] !== 2'bx1) begin
      $display("Failed cr[s3-:2], expected 2'bx1, got %b", cr[s3-:2]);
      passed = 1'b0;
    end

    $display("dr[s3-:2]: %b", dr[s3-:2]);
    if (dr[s3-:2] !== 2'bx0) begin
      $display("Failed dr[s3-:2], expected 2'bx0, got %b", dr[s3-:2]);
      passed = 1'b0;
    end

    $display("cr[s4-:2]: %b", cr[s4-:2]);
    if (cr[s4-:2] !== 2'bx1) begin
      $display("Failed cr[s4-:2], expected 2'bx1, got %b", cr[s4-:2]);
      passed = 1'b0;
    end

    $display("dr[s4-:2]: %b", dr[s4-:2]);
    if (dr[s4-:2] !== 2'bx0) begin
      $display("Failed dr[s4-:2], expected 2'bx0, got %b", dr[s4-:2]);
      passed = 1'b0;
    end

    // Check continuous assignment R-value variable index down selects.

    if (res_a0 !== 2'b0x) begin
      $display("Failed res_a0, expected 2'b0x, got %b", res_a0);
      passed = 1'b0;
    end

    if (res_b0 !== 2'b1x) begin
      $display("Failed res_b0, expected 2'b1x, got %b", res_b0);
      passed = 1'b0;
    end

    if (res_a1 !== 2'b0x) begin
      $display("Failed res_a1, expected 2'b0x, got %b", res_a1);
      passed = 1'b0;
    end

    if (res_b1 !== 2'b1x) begin
      $display("Failed res_b1, expected 2'b1x, got %b", res_b1);
      passed = 1'b0;
    end

    if (res_a2 !== 2'b0x) begin
      $display("Failed res_a2, expected 2'b0x, got %b", res_a2);
      passed = 1'b0;
    end

    if (res_b2 !== 2'b1x) begin
      $display("Failed res_b2, expected 2'b1x, got %b", res_b2);
      passed = 1'b0;
    end

    if (res_c3 !== 2'bx1) begin
      $display("Failed res_c3, expected 2'bx1, got %b", res_c3);
      passed = 1'b0;
    end

    if (res_d3 !== 2'bx0) begin
      $display("Failed res_d3, expected 2'bx0, got %b", res_d3);
      passed = 1'b0;
    end

    if (res_c4 !== 2'bx1) begin
      $display("Failed res_c4, expected 2'bx1, got %b", res_c4);
      passed = 1'b0;
    end

    if (res_d4 !== 2'bx0) begin
      $display("Failed res_d4, expected 2'bx0, got %b", res_d4);
      passed = 1'b0;
    end

    // Check procedural L-value variable index down selects.

    res_ab = 4'bxxxx;
    res_ab[s0-:2] = 2'b00;
    if (res_ab !== 4'bxxx0) begin
      $display("Failed res_ab[s0], expected 4'bxxx0, got %b", res_ab);
      passed = 1'b0;
    end

    res_ab = 4'bxxxx;
    res_ab[s1-:2] = 2'b00;
    if (res_ab !== 4'bxxx0) begin
      $display("Failed res_ab[s1], expected 4'bxxx0, got %b", res_ab);
      passed = 1'b0;
    end

    res_ab = 4'bxxxx;
    res_ab[s2-:2] = 2'b00;
    if (res_ab !== 4'bxxx0) begin
      $display("Failed res_ab[s2], expected 4'bxxx0, got %b", res_ab);
      passed = 1'b0;
    end

    res_cd = 4'bxxxx;
    res_cd[s3-:2] = 2'b00;
    if (res_cd !== 4'b0xxx) begin
      $display("Failed res_cd[s3], expected 4'b0xxx, got %b", res_cd);
      passed = 1'b0;
    end

    res_cd = 4'bxxxx;
    res_cd[s4-:2] = 2'b00;
    if (res_cd !== 4'b0xxx) begin
      $display("Failed res_cd[s4], expected 4'b0xxx, got %b", res_cd);
      passed = 1'b0;
    end

    if (passed) $display("Compare tests passed");
  end
endmodule
