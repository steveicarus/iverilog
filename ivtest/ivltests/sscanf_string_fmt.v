/*
 * Regression test for an uninitialized pointer read in $sscanf()/$fscanf()
 * when the format argument is a SystemVerilog string variable.
 *
 * scan_format() requested the format item as a vpiVectorVal to scan it for
 * X/Z bits, but a string variable does not provide a vector value, so the
 * value.vector pointer was left uninitialized and then dereferenced. With a
 * string literal the format worked, but holding the same format in a string
 * variable read stale stack memory and could crash or make the match fail.
 */
module top;
  parameter string pfmt = "%h";
  string fmt;
  integer cnt;
  reg [31:0] a1, a2, a3;

  initial begin
    // Single conversion through a string variable.
    fmt = "%d";
    a1 = 'bx;
    cnt = $sscanf("42", fmt, a1);
    if (cnt !== 1 || a1 !== 42) begin
      $display("FAILED -- single: cnt=%0d a1=%0d", cnt, a1);
      $finish;
    end

    // Empty string format must not match and must not crash.
    fmt = "";
    cnt = $sscanf("42", fmt, a1);
    if (cnt !== 0) begin
      $display("FAILED -- empty: cnt=%0d", cnt);
      $finish;
    end

    // Multiple conversions with the same format held in a string variable
    // as a literal would produce.
    fmt = "%d hex %h bin %b";
    a1 = 'bx; a2 = 'bx; a3 = 'bx;
    cnt = $sscanf("123 hex beaf bin 01xz", fmt, a1, a2, a3);
    if (cnt !== 3 || a1 !== 123 || a2 !== 'hbeaf || a3 !== 'b01xz) begin
      $display("FAILED -- multi: cnt=%0d a1=%0d a2=%h a3=%b", cnt, a1, a2, a3);
      $finish;
    end

    // A string parameter does provide a vector value, so it takes the
    // normal X/Z pre-scan path.
    a1 = 'bx;
    cnt = $sscanf("beaf", pfmt, a1);
    if (cnt !== 1 || a1 !== 'hbeaf) begin
      $display("FAILED -- param: cnt=%0d a1=%h", cnt, a1);
      $finish;
    end

    $display("PASSED");
  end
endmodule
