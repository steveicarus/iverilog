`timescale 1ns/10ps
module top;
  reg pass;
  reg [60*8-1:0] str, cmp;
  reg [7:0] bval;
  reg [15:0] oval, hval;
  integer dval;
  time tval;
  real rval;

  initial begin
    pass = 1'b1;

    // Check the %b conversion.
    bval = 8'b01101001;
    cmp = "1101001";
    $sformat(str, "%0b", bval);
    if (str != cmp) begin
      $display("FAILED: %%0b, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "0000000001101001";
    $sformat(str, "%016b", bval);
    if (str != cmp) begin
      $display("FAILED: %%016b, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "1101001         ";
    $sformat(str, "%-016b", bval);
    if (str != cmp) begin
      $display("FAILED: %%-016b, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    // Check the %o conversion.
    oval = 16'o01234;
    cmp = "1234";
    $sformat(str, "%0o", oval);
    if (str != cmp) begin
      $display("FAILED: %%0o, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "00001234";
    $sformat(str, "%08o", oval);
    if (str != cmp) begin
      $display("FAILED: %%08o, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "1234    ";
    $sformat(str, "%-08o", oval);
    if (str != cmp) begin
      $display("FAILED: %%-08o, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    // Check the %h conversion.
    hval = 16'h0abc;
    cmp = "abc";
    $sformat(str, "%0h", hval);
    if (str != cmp) begin
      $display("FAILED: %%0h, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "00000abc";
    $sformat(str, "%08h", hval);
    if (str != cmp) begin
      $display("FAILED: %%08h, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "abc     ";
    $sformat(str, "%-08h", hval);
    if (str != cmp) begin
      $display("FAILED: %%-08h, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    // Check the %c conversion.
    bval = "c";
    cmp = "c";
    $sformat(str, "%0c", bval);
    if (str != cmp) begin
      $display("FAILED: %%0c, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "000c";
    $sformat(str, "%04c", bval);
    if (str != cmp) begin
      $display("FAILED: %%04c, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    // Check the %d conversion.
    dval = 123;
    cmp = "00000123";
    $sformat(str, "%08d", dval);
    if (str != cmp) begin
      $display("FAILED: %%08d, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "+0000123";
    $sformat(str, "%+08d", dval);
    if (str != cmp) begin
      $display("FAILED: %%+08d, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "        123";
    $sformat(str, "%d", dval);
    if (str != cmp) begin
      $display("FAILED: %%d, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "123     ";
    $sformat(str, "%-08d", dval);
    if (str != cmp) begin
      $display("FAILED: %%-08d, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "123";
    $sformat(str, "%0d", dval);
    if (str != cmp) begin
      $display("FAILED: %%0d, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    dval = -123;
    cmp = "-0000123";
    $sformat(str, "%08d", dval);
    if (str != cmp) begin
      $display("FAILED: %%08d, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    $sformat(str, "%+08d", dval);
    if (str != cmp) begin
      $display("FAILED: %%+08d, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "       -123";
    $sformat(str, "%d", dval);
    if (str != cmp) begin
      $display("FAILED: %%d, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    // Check the %t conversion.
    tval = 100_000;
    cmp = "0010000000";
    $sformat(str, "%010t", tval);
    if (str != cmp) begin
      $display("FAILED: %%010t, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "            10000000"; // Default width is 20.
    $sformat(str, "%t", tval);
    if (str != cmp) begin
      $display("FAILED: %%t, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "10000000  ";
    $sformat(str, "%-010t", tval);
    if (str != cmp) begin
      $display("FAILED: %%-010t, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "10000000";
    $sformat(str, "%0t", tval);
    if (str != cmp) begin
      $display("FAILED: %%0t, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    rval=100_000.25;
    cmp = "0010000025";
    $sformat(str, "%010t", rval);
    if (str != cmp) begin
      $display("FAILED: %%010t (real), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "            10000025"; // Default width is 20.
    $sformat(str, "%t", rval);
    if (str != cmp) begin
      $display("FAILED: %%t (real), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "10000025  ";
    $sformat(str, "%-010t", rval);
    if (str != cmp) begin
      $display("FAILED: %%-010t (real), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "10000025";
    $sformat(str, "%0t", rval);
    if (str != cmp) begin
      $display("FAILED: %%0t (real), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    // Display in ns with 10ps resolution.
    $timeformat(-9, 2, " ns", 15);

    cmp = "000100000.00 ns";
    $sformat(str, "%015t", tval);
    if (str != cmp) begin
      $display("FAILED: %%015t (w/$tf), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "   100000.00 ns";
    $sformat(str, "%t", tval);
    if (str != cmp) begin
      $display("FAILED: %%t (w/$tf), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "100000.00 ns   ";
    $sformat(str, "%-015t", tval);
    if (str != cmp) begin
      $display("FAILED: %%-015t (w/$tf), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "100000.00 ns";
    $sformat(str, "%-0t", tval);
    if (str != cmp) begin
      $display("FAILED: %%-0t (w/$tf), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "000100000.25 ns";
    $sformat(str, "%015t", rval);
    if (str != cmp) begin
      $display("FAILED: %%015t (w/$tf, rl), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "   100000.25 ns";
    $sformat(str, "%t", rval);
    if (str != cmp) begin
      $display("FAILED: %%t (w/$tf, rl), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "100000.25 ns   ";
    $sformat(str, "%-015t", rval);
    if (str != cmp) begin
      $display("FAILED: %%-015t (w/$tf, rl), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "100000.25 ns";
    $sformat(str, "%-0t", rval);
    if (str != cmp) begin
      $display("FAILED: %%-0t (w/$tf, rl), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    // Check the real conversions (%e, %f, %g). If one works they all
    // they all work (uses system conversion).

    rval = 1.25;
    cmp = "000000001.250000";
    $sformat(str, "%016.6f", rval);
    if (str != cmp) begin
      $display("FAILED: %%016.6f, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "+00000001.250000";
    $sformat(str, "%+016.6f", rval);
    if (str != cmp) begin
      $display("FAILED: %%+016.6f, expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    rval = -1.25;
    cmp = "-00000001.250000";
    $sformat(str, "%016.6f", rval);
    if (str != cmp) begin
      $display("FAILED: %%016.6f (negative), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

    cmp = "-00000001.250000";
    $sformat(str, "%+016.6f", rval);
    if (str != cmp) begin
      $display("FAILED: %%+016.6f (negative), expected %0s, got %0s", cmp, str);
      pass = 1'b0;
    end

     if (pass) $display("PASSED");
  end
endmodule
