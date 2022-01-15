// Check the various variable bit selects (MSB > LSB).
module top;
  parameter [4:1] ap = 4'h8;
  parameter [4:1] bp = 4'h7;
  parameter [0:-3] cp = 4'h8;
  parameter [0:-3] dp = 4'h7;

  reg passed;

  wire [4:1] a = 4'h8;
  wire [4:1] b = 4'h7;
  wire [0:0] s0 = 0;
  wire [1:0] s1 = 0;
  wire [2:0] s2 = 0;
  reg [4:1] ar = 4'h8;
  reg [4:1] br = 4'h7;

  wire [0:-3] c = 4'h8;
  wire [0:-3] d = 4'h7;
  wire [0:0] s3 = 0;
  wire [1:0] s4 = 0;
  reg [0:-3] cr = 4'h8;
  reg [0:-3] dr = 4'h7;

  wire res_a0 = a[s0];
  wire res_b0 = b[s0];
  wire res_a1 = a[s1];
  wire res_b1 = b[s1];
  wire res_a2 = a[s2];
  wire res_b2 = b[s2];

  wire res_c3 = c[s3];
  wire res_d3 = d[s3];
  wire res_c4 = c[s4];
  wire res_d4 = d[s4];

  reg [4:1] res_ab;
  reg [0:-3] res_cd;

  initial begin
    #1;
    passed = 1'b1;

    // Check procedural R-value variable bit selects of a net.

    $display("a[s0]: %b", a[s0]);
    if (a[s0] !== 1'bx) begin
      $display("Failed a[s0], expected 1'bx, got %b", a[s0]);
      passed = 1'b0;
    end

    $display("b[s0]: %b", b[s0]);
    if (b[s0] !== 1'bx) begin
      $display("Failed b[s0], expected 1'bx, got %b", b[s0]);
      passed = 1'b0;
    end

    $display("a[s1]: %b", a[s1]);
    if (a[s1] !== 1'bx) begin
      $display("Failed a[s1], expected 1'bx, got %b", a[s1]);
      passed = 1'b0;
    end

    $display("b[s1]: %b", b[s1]);
    if (b[s1] !== 1'bx) begin
      $display("Failed b[s1], expected 1'bx, got %b", b[s1]);
      passed = 1'b0;
    end

    $display("a[s2]: %b", a[s2]);
    if (a[s2] !== 1'bx) begin
      $display("Failed a[s2], expected 1'bx, got %b", a[s2]);
      passed = 1'b0;
    end

    $display("b[s2]: %b", b[s2]);
    if (b[s2] !== 1'bx) begin
      $display("Failed b[s2], expected 1'bx, got %b", b[s2]);
      passed = 1'b0;
    end

    $display("c[s3]: %b", c[s3]);
    if (c[s3] !== 1'b1) begin
      $display("Failed c[s3], expected 1'b1, got %b", c[s3]);
      passed = 1'b0;
    end

    $display("d[s3]: %b", d[s3]);
    if (d[s3] !== 1'b0) begin
      $display("Failed d[s3], expected 1'b0, got %b", d[s3]);
      passed = 1'b0;
    end

    $display("c[s4]: %b", c[s4]);
    if (c[s4] !== 1'b1) begin
      $display("Failed c[s4], expected 1'b1, got %b", c[s4]);
      passed = 1'b0;
    end

    $display("d[s4]: %b", d[s4]);
    if (d[s4] !== 1'b0) begin
      $display("Failed d[s4], expected 1'b0, got %b", d[s4]);
      passed = 1'b0;
    end

    // Check procedural R-value variable bit selects of a parameter.

    $display("ap[s0]: %b", ap[s0]);
    if (ap[s0] !== 1'bx) begin
      $display("Failed ap[s0], expected 1'bx, got %b", ap[s0]);
      passed = 1'b0;
    end

    $display("bp[s0]: %b", bp[s0]);
    if (bp[s0] !== 1'bx) begin
      $display("Failed bp[s0], expected 1'bx, got %b", bp[s0]);
      passed = 1'b0;
    end

    $display("ap[s1]: %b", ap[s1]);
    if (ap[s1] !== 1'bx) begin
      $display("Failed ap[s1], expected 1'bx, got %b", ap[s1]);
      passed = 1'b0;
    end

    $display("bp[s1]: %b", bp[s1]);
    if (bp[s1] !== 1'bx) begin
      $display("Failed bp[s1], expected 1'bx, got %b", bp[s1]);
      passed = 1'b0;
    end

    $display("ap[s2]: %b", ap[s2]);
    if (ap[s2] !== 1'bx) begin
      $display("Failed ap[s2], expected 1'bx, got %b", ap[s2]);
      passed = 1'b0;
    end

    $display("bp[s2]: %b", bp[s2]);
    if (bp[s2] !== 1'bx) begin
      $display("Failed bp[s2], expected 1'bx, got %b", bp[s2]);
      passed = 1'b0;
    end

    $display("cp[s3]: %b", cp[s3]);
    if (cp[s3] !== 1'b1) begin
      $display("Failed cp[s3], expected 1'b1, got %b", cp[s3]);
      passed = 1'b0;
    end

    $display("dp[s3]: %b", dp[s3]);
    if (dp[s3] !== 1'b0) begin
      $display("Failed dp[s3], expected 1'b0, got %b", dp[s3]);
      passed = 1'b0;
    end

    $display("cp[s4]: %b", cp[s4]);
    if (cp[s4] !== 1'b1) begin
      $display("Failed cp[s4], expected 1'b1, got %b", cp[s4]);
      passed = 1'b0;
    end

    $display("dp[s4]: %b", dp[s4]);
    if (dp[s4] !== 1'b0) begin
      $display("Failed dp[s4], expected 1'b0, got %b", dp[s4]);
      passed = 1'b0;
    end

    // Check procedural R-value variable bit selects of a reg.

    $display("ar[s0]: %b", ar[s0]);
    if (ar[s0] !== 1'bx) begin
      $display("Failed ar[s0], expected 1'bx, got %b", ar[s0]);
      passed = 1'b0;
    end

    $display("br[s0]: %b", br[s0]);
    if (br[s0] !== 1'bx) begin
      $display("Failed br[s0], expected 1'bx, got %b", br[s0]);
      passed = 1'b0;
    end

    $display("ar[s1]: %b", ar[s1]);
    if (ar[s1] !== 1'bx) begin
      $display("Failed ar[s1], expected 1'bx, got %b", ar[s1]);
      passed = 1'b0;
    end

    $display("br[s1]: %b", br[s1]);
    if (br[s1] !== 1'bx) begin
      $display("Failed br[s1], expected 1'bx, got %b", br[s1]);
      passed = 1'b0;
    end

    $display("ar[s2]: %b", ar[s2]);
    if (ar[s2] !== 1'bx) begin
      $display("Failed ar[s2], expected 1'bx, got %b", ar[s2]);
      passed = 1'b0;
    end

    $display("br[s2]: %b", br[s2]);
    if (br[s2] !== 1'bx) begin
      $display("Failed br[s2], expected 1'bx, got %b", br[s2]);
      passed = 1'b0;
    end

    $display("cr[s3]: %b", cr[s3]);
    if (cr[s3] !== 1'b1) begin
      $display("Failed cr[s3], expected 1'b1, got %b", cr[s3]);
      passed = 1'b0;
    end

    $display("dr[s3]: %b", dr[s3]);
    if (dr[s3] !== 1'b0) begin
      $display("Failed dr[s3], expected 1'b0, got %b", dr[s3]);
      passed = 1'b0;
    end

    $display("cr[s4]: %b", cr[s4]);
    if (cr[s4] !== 1'b1) begin
      $display("Failed cr[s4], expected 1'b1, got %b", cr[s4]);
      passed = 1'b0;
    end

    $display("dr[s4]: %b", dr[s4]);
    if (dr[s4] !== 1'b0) begin
      $display("Failed dr[s4], expected 1'b0, got %b", dr[s4]);
      passed = 1'b0;
    end

    // Check continuous assignment R-value variable bit selects.

    if (res_a0 !== 1'bx) begin
      $display("Failed res_a0, expected 1'bx, got %b", res_a0);
      passed = 1'b0;
    end

    if (res_b0 !== 1'bx) begin
      $display("Failed res_b0, expected 1'bx, got %b", res_b0);
      passed = 1'b0;
    end

    if (res_a1 !== 1'bx) begin
      $display("Failed res_a1, expected 1'bx, got %b", res_a1);
      passed = 1'b0;
    end

    if (res_b1 !== 1'bx) begin
      $display("Failed res_b1, expected 1'bx, got %b", res_b1);
      passed = 1'b0;
    end

    if (res_a2 !== 1'bx) begin
      $display("Failed res_a2, expected 1'bx, got %b", res_a2);
      passed = 1'b0;
    end

    if (res_b2 !== 1'bx) begin
      $display("Failed res_b2, expected 1'bx, got %b", res_b2);
      passed = 1'b0;
    end

    if (res_c3 !== 1'b1) begin
      $display("Failed res_c3, expected 1'b1, got %b", res_c3);
      passed = 1'b0;
    end

    if (res_d3 !== 1'b0) begin
      $display("Failed res_d3, expected 1'b0, got %b", res_d3);
      passed = 1'b0;
    end

    if (res_c4 !== 1'b1) begin
      $display("Failed res_c4, expected 1'b1, got %b", res_c4);
      passed = 1'b0;
    end

    if (res_d4 !== 1'b0) begin
      $display("Failed res_d4, expected 1'b0, got %b", res_d4);
      passed = 1'b0;
    end

    // Check procedural L-value variable bit selects.

    res_ab = 4'bxxxx;
    res_ab[s0] = 1'b0;
    if (res_ab !== 4'bxxxx) begin
      $display("Failed res_ab[s0], expected 4'bxxxx, got %b", res_ab);
      passed = 1'b0;
    end

    res_ab = 4'bxxxx;
    res_ab[s1] = 1'b0;
    if (res_ab !== 4'bxxxx) begin
      $display("Failed res_ab[s1], expected 4'bxxxx, got %b", res_ab);
      passed = 1'b0;
    end

    res_ab = 4'bxxxx;
    res_ab[s2] = 1'b0;
    if (res_ab !== 4'bxxxx) begin
      $display("Failed res_ab[s2], expected 4'bxxxx, got %b", res_ab);
      passed = 1'b0;
    end

    res_cd = 4'bxxxx;
    res_cd[s3] = 1'b0;
    if (res_cd !== 4'b0xxx) begin
      $display("Failed res_cd[s3], expected 4'b0xxx, got %b", res_cd);
      passed = 1'b0;
    end

    res_cd = 4'bxxxx;
    res_cd[s4] = 1'b0;
    if (res_cd !== 4'b0xxx) begin
      $display("Failed res_cd[s4], expected 4'b0xxx, got %b", res_cd);
      passed = 1'b0;
    end

    if (passed) $display("Compare tests passed");
  end
endmodule
