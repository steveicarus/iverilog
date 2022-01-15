// Explore how procedural concatenations work in various contexts.
// Some of the checks are specific to the 1364-2005 standard.
//
// Cary R. cygcary@yahoo.com

module main;
  reg pass;
  reg [31:0] a_c, b_c, c_c, d_c, a_r, b_r, c_r, d_r;
  reg [127:0] y_c;
  integer seed, fres;
  // These will have the following value depending on the order.
  // 1 = LSB->MSB, 2 = MSB->LSB, 3 = indeterminate.
  integer sorder, uorder;

  initial begin
    pass = 1'b1;

    /**********
     * Try to find the order using $random.
     **********/
    // Start from a known place.
    seed = 0;
    y_c = {$random(seed), $random(seed), $random(seed), $random(seed)};
    a_c = y_c[31:0];
    b_c = y_c[63:32];
    c_c = y_c[95:64];
    d_c = y_c[127:96];

    // Make the reference values in a known order.
    seed = 0;
    a_r = $random(seed);
    b_r = $random(seed);
    c_r = $random(seed);
    d_r = $random(seed);

    if (a_c === a_r && b_c === b_r && c_c === c_r && d_c == d_r) begin
      $display("Concatenation of system functions is LSB -> MSB.");
      sorder = 1;
    end else if (a_c === d_r && b_c === c_r && c_c === b_r && d_c == a_r) begin
      $display("Concatenation of system functions is MSB -> LSB.");
      sorder = 2;
    end else if ((a_c === a_r || a_c === b_r || a_c === c_r || a_c == d_r) &&
                 (b_c === a_r || b_c === b_r || b_c === c_r || b_c == d_r) &&
                 (c_c === a_r || c_c === b_r || c_c === c_r || c_c == d_r) &&
                 (d_c === a_r || d_c === b_r || d_c === c_r || d_c == d_r))
    begin
      $display("Concatenation of system functions is indeterminate.");
      $display("  check:",, d_c,, c_c,, b_c,, a_c);
      $display("   ref.:",, d_r,, c_r,, b_r,, a_r);
      sorder = 3;
    end else begin
      $display("FAILED: system function concatenation order.");
      $display("  check:",, d_c,, c_c,, b_c,, a_c);
      $display("   ref.:",, d_r,, c_r,, b_r,, a_r);
      pass = 1'b0;
    end

    /**********
     * Try to find the order using ufunc().
     **********/
    // Start from a known place.
    fres = 0;
    y_c = {ufunc(0), ufunc(0), ufunc(0), ufunc(0)};
    a_c = y_c[31:0];
    b_c = y_c[63:32];
    c_c = y_c[95:64];
    d_c = y_c[127:96];

    // Make the reference values in a known order.
    fres = 0;
    a_r = ufunc(0);
    b_r = ufunc(0);
    c_r = ufunc(0);
    d_r = ufunc(0);

    if (a_c === a_r && b_c === b_r && c_c === c_r && d_c == d_r) begin
      $display("Concatenation of user functions is LSB -> MSB.");
      uorder = 1;
    end else if (a_c === d_r && b_c === c_r && c_c === b_r && d_c == a_r) begin
      $display("Concatenation of user functions is MSB -> LSB.");
      uorder = 2;
    end else if ((a_c === a_r || a_c === b_r || a_c === c_r || a_c == d_r) &&
                 (b_c === a_r || b_c === b_r || b_c === c_r || b_c == d_r) &&
                 (c_c === a_r || c_c === b_r || c_c === c_r || c_c == d_r) &&
                 (d_c === a_r || d_c === b_r || d_c === c_r || d_c == d_r))
    begin
      $display("Concatenation of user functions is indeterminate.");
      $display("  check:",, d_c,, c_c,, b_c,, a_c);
      $display("   ref.:",, d_r,, c_r,, b_r,, a_r);
      uorder = 3;
    end else begin
      $display("FAILED: user function concatenation order.");
      $display("  check:",, d_c,, c_c,, b_c,, a_c);
      $display("   ref.:",, d_r,, c_r,, b_r,, a_r);
      pass = 1'b0;
    end

    if (sorder != uorder) begin
      $display("WARNING: system functions and user functions have a ",
               "different order.");
    end

    /**********
     * Check to see if extra system functions are called and ignored.
     * We do not care about the order for this test.
     **********/
    // Start from a known place.
    seed = 0;
    // You must run the extra $random(), but drop the result.
    c_c = {$random(seed), $random(seed), $random(seed), $random(seed),
           $random(seed), $random(seed)};
    a_c = $random(seed);

    // Make the reference values in a known order.
    seed = 0;
    a_r = $random(seed);
    a_r = $random(seed);
    a_r = $random(seed);
    a_r = $random(seed);
    a_r = $random(seed);
    a_r = $random(seed);
    a_r = $random(seed);

    if (a_c !== a_r) begin
      $display("FAILED: extra system functions in a concat. are not run.");
      pass = 1'b0;
    end

    /**********
     * Check to see if extra user functions are called and ignored.
     * We do not care about the order for this test.
     **********/
    // Start from a known place.
    fres = 0;
    y_c = {ufunc(0), ufunc(0), ufunc(0), ufunc(0), ufunc(0), ufunc(0)};

    if (fres != 6) begin
      $display("FAILED: extra user functions in a concat. are not run.");
      pass = 1'b0;
    end

    // Icarus handles this in a different way so check it as well.
    // Start from a known place.
    fres = 0;
    y_c = check_64({ufunc(0), ufunc(0), ufunc(0)});
    if (fres != 3) begin
      $display("FAILED: extra ufunc in a concat. as an argument are not run.");
      pass = 1'b0;
    end

    /**********
     * Check to see if a system function replicated 0 times is done correctly.
     **********/
    // Start from a known place.
    seed = 0;
    // You must run the zero replication system call and then ignore it.
    a_c = {{0{$random(seed)}}, $random(seed)};
    a_c = $random(seed);

    // Make a reference value.
    seed = 0;
    a_r = $random(seed);
    a_r = $random(seed);
    a_r = $random(seed);

    if (a_c !== a_r) begin
      $display("FAILED: zero repl. system functions in a concat. are not run.");
      pass = 1'b0;
    end

    /**********
     * Check to see if a user function replicated 0 times is done correctly.
     **********/
    // Start from a known place.
    fres = 0;
    // You must run the zero replication user call and then ignore it.
    a_c = {{0{ufunc(0)}}, ufunc(0)};

    if (fres != 2) begin
      $display("FAILED: zero repl. user functions in a concat. are not run.");
      pass = 1'b0;
    end

    /**********
     * Check a simple replication of $random().
     **********/
    // Start from a known place.
    seed = 0;
    // This must run $random() only once.
    y_c = {4{$random(seed)}};
    a_c = y_c[31:0];
    b_c = y_c[63:32];
    c_c = y_c[95:64];
    d_c = y_c[127:96];

    // Start from a known place.
    seed = 0;
    a_r = $random(seed);
    b_r = a_r;
    c_r = a_r;
    d_r = a_r;

    if (a_c !== a_r || b_c !== b_r || c_c !== c_r || d_c !== d_r) begin
      $display("FAILED $random() replication, each replication is different.");
      pass = 1'b0;
    end

    /**********
     * Check a replication of ufunc().
     **********/
    // Start from a known place.
    fres = 0;
    // This must run ufunc() only once.
    y_c = {4{ufunc(0)}};
    a_c = y_c[31:0];
    b_c = y_c[63:32];
    c_c = y_c[95:64];
    d_c = y_c[127:96];

    // Start from a known place.
    fres = 0;
    a_r = ufunc(0);
    b_r = a_r;
    c_r = a_r;
    d_r = a_r;

    if (a_c !== a_r || b_c !== b_r || c_c !== c_r || d_c !== d_r) begin
      $display("FAILED ufunc() replication, each replication is different.");
      pass = 1'b0;
    end

    /*
     * A concatenation as an argument needs to pad or select as needed.
     * We only check ufunc since it should be the same as $random and
     * it has been tested above.
     */

    /**********
     * Check that a concat is zero extended.
     **********/
    // Start from a known place.
    fres = 0;
    y_c = check_64({ufunc(1)});
    a_c = y_c[31:0];
    b_c = y_c[63:32];
    c_c = y_c[95:64];
    d_c = y_c[127:96];

    // Start from a known place.
    fres = 0;
    a_r = ufunc(1);
    b_r = 32'h0;
    c_r = 32'h0;
    d_r = 32'h0;

    if (a_c !== a_r || b_c !== b_r || c_c !== c_r || d_c !== d_r) begin
      $display("FAILED padded user function concatenation.");
      $displayh("  check:",, d_c,, c_c,, b_c,, a_c);
      $displayh("   ref.:",, d_r,, c_r,, b_r,, a_r);
      pass = 1'b0;
    end

    /**********
     * Check that a $signed concat is sign extended.
     **********/
    // Start from a known place.
    fres = 0;
    y_c = check_64($signed({ufunc(1)}));
    a_c = y_c[31:0];
    b_c = y_c[63:32];
    c_c = y_c[95:64];
    d_c = y_c[127:96];

    // Start from a known place.
    fres = 0;
    a_r = ufunc(1);
    b_r = 32'hffffffff;
    c_r = 32'h0;
    d_r = 32'h0;

    if (a_c !== a_r || b_c !== b_r || c_c !== c_r || d_c !== d_r) begin
      $display("FAILED sign padded user function concatenation.");
      $displayh("  check:",, d_c,, c_c,, b_c,, a_c);
      $displayh("   ref.:",, d_r,, c_r,, b_r,, a_r);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

  function [63:0] check_64;
    input [63:0] in;
    check_64 = in;
  endfunction

  // This user function has a side effect (fres) so the result is call
  // order dependent.
  function integer ufunc;
    input in;
    begin
      if (in) fres = fres - 1;
      else fres = fres + 1;
        ufunc = fres;
    end
  endfunction
endmodule
