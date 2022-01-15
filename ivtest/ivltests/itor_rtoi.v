/*
 * This does not check that $itor() and $rtoi() can actually be called in
 * a constant context (e.g. a parameter assignment). This was done so that
 * older version of Icarus Verilog will still run this code to verify that
 * the values are converted correctly. Also some of the results are not
 * defined in the standard so we use a gold file instead of pass/fail test
 * to make it easier to check other simulators.
 */
module top;
  integer ival;
  real rval;

  initial begin
    $display("Testing $itor() in a constant context.");
    // Check various integer values.
    rval = $itor(10); $display("  $itor(10)   = %g", rval);
    rval = $itor(1'bx); $display("  $itor(1'bx) = %g", rval);
    rval = $itor(1'bz); $display("  $itor(1'bx) = %g", rval);
    // Check various real values.
    rval = $itor(10.4); $display("  $itor(10.4) = %g", rval);
    rval = $itor(10.5); $display("  $itor(10.5) = %g", rval);
    rval = $itor(-1.4); $display("  $itor(-1.4) = %g", rval);
    rval = $itor(-1.5); $display("  $itor(-1.5) = %g", rval);
    rval = $itor(0.0/0.0); $display("  $itor(NaN)  = %g", rval);
    rval = $itor(1.0/0.0); $display("  $itor(+inf) = %g", rval);
    rval = $itor(-1.0/0.0); $display("  $itor(-inf) = %g", rval);

    $display("");
    $display("Testing $itor() in a variable context.");
    // Check various integer values.
    ival = 10; rval = $itor(ival); $display("  $itor(10)   = %g", rval);
    ival = 1'bx; rval = $itor(ival); $display("  $itor(1'bx) = %g", rval);
    ival = 1'bx; rval = $itor(ival); $display("  $itor(1'bx) = %g", rval);
    // Check various real values.
    rval = 10.4; rval = $itor(rval); $display("  $itor(10.4) = %g", rval);
    rval = 10.5; rval = $itor(rval); $display("  $itor(10.5) = %g", rval);
    rval = -1.4; rval = $itor(rval); $display("  $itor(-1.4) = %g", rval);
    rval = -1.5; rval = $itor(rval); $display("  $itor(-1.5) = %g", rval);
    rval = 0.0/0.0; rval = $itor(rval); $display("  $itor(NaN)  = %g", rval);
    rval = 1.0/0.0; rval = $itor(rval); $display("  $itor(+inf) = %g", rval);
    rval = -1.0/0.0; rval = $itor(rval); $display("  $itor(-inf) = %g", rval);

    $display("");
    $display("Testing $rtoi() in a constant context.");
    // Check for truncation of a positive value.
    ival = $rtoi(1.1); $display("  $rtoi(1.1)  = %0d", ival);
    ival = $rtoi(1.9); $display("  $rtoi(1.9)  = %0d", ival);
    // Check for truncation of a negative value.
    ival = $rtoi(-1.1); $display("  $rtoi(-1.1) = %0d", ival);
    ival = $rtoi(-1.9); $display("  $rtoi(-1.9) = %0d", ival);
    // Check a value larger than an integer is truncated.
    ival = $rtoi((33'b1<<32)+0.0); $display("  Overflow(0) = %0d", ival);
    ival = $rtoi((33'b1<<32)+1.0); $display("  Overflow(1) = %0d", ival);
    // Check NaN, +/- infinity.
    ival = $rtoi(0.0/0.0); $display("  $rtoi(NaN)  = %0d", ival);
    ival = $rtoi(1.0/0.0); $display("  $rtoi(+inf) = %0d", ival);
    ival = $rtoi(-1.0/0.0); $display("  $rtoi(-inf) = %0d", ival);
    // Check various integer values.
    ival = $rtoi(1); $display("  $rtoi(1)    = %0d", ival);
    ival = $rtoi(1'bx); $display("  $rtoi(1'bx) = %0d", ival);
    ival = $rtoi(1'bz); $display("  $rtoi(1'bz) = %0d", ival);

    $display("");
    $display("Testing $rtoi() in a variable context.");
    // Check for truncation of a positive value.
    rval = 1.1; ival = $rtoi(rval); $display("  $rtoi(1.1)  = %0d", ival);
    rval = 1.9; ival = $rtoi(rval); $display("  $rtoi(1.9)  = %0d", ival);
    // Check for truncation of a negative value.
    rval = -1.1; ival = $rtoi(rval); $display("  $rtoi(-1.1) = %0d", ival);
    rval = -1.9; ival = $rtoi(rval); $display("  $rtoi(-1.9) = %0d", ival);
    // Check a value larger than an integer is truncated.
    rval = (33'b1<<32)+0.0; ival = $rtoi(rval); $display("  Overflow(0) = %0d", ival);
    rval = (33'b1<<32)+1.0; ival = $rtoi(rval); $display("  Overflow(1) = %0d", ival);
    // Check NaN, +/- infinity.
    rval = 0.0/0.0; ival = $rtoi(rval); $display("  $rtoi(NaN)  = %0d", ival);
    rval = 1.0/0.0; ival = $rtoi(rval); $display("  $rtoi(+inf) = %0d", ival);
    rval = -1.0/0.0; ival = $rtoi(rval); $display("  $rtoi(-inf) = %0d", ival);
    // Check various integer values.
    ival = 1; ival = $rtoi(ival); $display("  $rtoi(1)    = %0d", ival);
    ival = 1'bx; ival = $rtoi(ival); $display("  $rtoi(1'bx) = %0d", ival);
    ival = 1'bz; ival = $rtoi(ival); $display("  $rtoi(1'bz) = %0d", ival);

  end
endmodule
