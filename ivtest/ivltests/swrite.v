`timescale 1ns/1ps

//`define DEBUG
module top;
  parameter length = 34;
  parameter str = "%s";
  reg [length*8-1:0] result, fmt;
  integer val = 1000;
  reg [31:0] eval, uval, zval;
  reg [63:0] hval, sval;
  real rval = 1234.567;
  wire net;
  time tm = 234567;
  realtime rtm = 2345.678;
  reg failed;
`ifdef DEBUG
  integer lp;
`endif

  assign (pull1, strong0) net = 1'b1;

  task check_result;
    input [length*8-1:0] result, value;
    input [80*8-1:0] message;
    if (result != value) begin
      $display("%0s", message);
      $display("Got    :%s:", result);
      $display("Wanted :%s:", value);
`ifdef DEBUG
      for (lp=0; lp<length; lp=lp+1) begin
        $display("%d - %d, %d", lp, result[lp*8 +: 8], value[lp*8 +: 8]);
      end
`endif
      failed = 1;
    end
  endtask

  initial begin
    $timeformat(-12, 4, " ps", 20);
    fmt = "%s";
    failed = 0;
    // Determine the endian order.
    $swrite(result, "%u", "Help");
    if (result != "\000Help") begin
      // Big endian so reverse the bytes.
      eval = 32'h22000000;
      hval = 64'h206d652148656c70;
      uval = 32'b010010xz_01100101_01101100_01110000;
      zval = 32'b01z01000_0xx0zx0x_0xx01x0z_01x1000z;
      sval = " me!Help";
    end else begin
      // Little endian.
      eval = 32'h00000022;
      hval = 64'h21656d20706c6548;
      uval = 32'b01110000_01101100_01100101_010010xz;
      zval = 32'b01x1000z_0xx01x0z_0xx0zx0x_01z01000;
      sval = "!em pleH";
    end

    #1;

    // Basic variables and functions.
    $swrite(result, val);
    check_result(result, "       1000", "Decimal in $swrite failed!");
    $swriteb(result, val);
    check_result(result, "00000000000000000000001111101000",
                 "Decimal in $swriteb failed!");
    $swriteo(result, val);
    check_result(result, "00000001750", "Decimal in $swriteo failed!");
    $swriteh(result, val);
    check_result(result, "000003e8", "Decimal in $swriteo failed!");
    $swrite(result, rval);
    check_result(result, "1234.57", "Real in $swrite failed!");
    $swrite(result, "Normal string.");
    check_result(result, "Normal string.", "String in $swrite failed!");
    $swrite(result, tm);
    check_result(result, "              234567", "Time in $swrite failed!");
    $swrite(result, rtm);
    check_result(result, "2345.68", "Real time in $swrite failed!");
    $swrite(result, $time);
    check_result(result, "                   1", "$time in $swrite failed!");
    $swrite(result, $stime);
    check_result(result, "         1", "$stime in $swrite failed!");
    $swrite(result, $simtime);
    check_result(result, "                1000", "$simtime in $swrite failed!");
    $swrite(result, $realtime);
    check_result(result, "1.000", "$realtime in $swrite failed!");

    // %% and extra variables.
    $swrite(result, "%%",, val);
    check_result(result, "%        1000", "% and value in $swrite failed!");

    // %b
    $swrite(result, "%b", net);
    check_result(result, "1", "%b in $swrite failed!");
    $swrite(result, "%B", net);
    check_result(result, "1", "%b in $swrite failed!");
    $swrite(result, "%b", 8'b00001001);
    check_result(result, "00001001", "%b in $swrite failed!");
    $swrite(result, "%0b", 8'b00001001);
    check_result(result, "1001", "%0b in $swrite failed!");
    $swrite(result, "%14b", 8'b00001001);
    check_result(result, "      00001001", "%14b in $swrite failed!");
    $swrite(result, "%-14b", 8'b00001001);
    check_result(result, "00001001      ", "%-14b in $swrite failed!");

    // %o
    $swrite(result, "%o", 8'b00001001);
    check_result(result, "011", "%o in $swrite failed!");
    $swrite(result, "%O", 8'b00001001);
    check_result(result, "011", "%O in $swrite failed!");
    $swrite(result, "%0o", 8'b00001001);
    check_result(result, "11", "%0o in $swrite failed!");
    $swrite(result, "%14o", 8'b00001001);
    check_result(result, "           011", "%14o in $swrite failed!");
    $swrite(result, "%-14o", 8'b00001001);
    check_result(result, "011           ", "%-14o in $swrite failed!");

    // %h
    $swrite(result, "%h", 8'b00001001);
    check_result(result, "09", "%h in $swrite failed!");
    $swrite(result, "%H", 8'b00001001);
    check_result(result, "09", "%H in $swrite failed!");
    $swrite(result, "%0h", 8'b00001001);
    check_result(result, "9", "%0h in $swrite failed!");
    $swrite(result, "%14h", 8'b00001001);
    check_result(result, "            09", "%14h in $swrite failed!");
    $swrite(result, "%-14h", 8'b00001001);
    check_result(result, "09            ", "%-14h in $swrite failed!");

    // %c
    $swrite(result, "%c", "abcd");
    check_result(result, "d", "%c in $swrite failed!");
    $swrite(result, "%C", "abcd");
    check_result(result, "d", "%C in $swrite failed!");
    $swrite(result, "%4c", "abcd");
    check_result(result, "   d", "%4c in $swrite failed!");
    $swrite(result, "%-4c", "abcd");
    check_result(result, "d   ", "%-4c in $swrite failed!");

    // %d
    $swrite(result, "%d", val);
    check_result(result, "       1000", "%d in $swrite failed!");
    $swrite(result, "%D", val);
    check_result(result, "       1000", "%D in $swrite failed!");
    $swrite(result, "%d", length);
`ifdef OLD_UNSIZED
    check_result(result, " 34", "%d in $swrite failed!");
`else
    check_result(result, "         34", "%d in $swrite failed!");
`endif
    $swrite(result, "%d", 31);
`ifdef OLD_UNSIZED
    check_result(result, " 31", "%d in $swrite failed!");
`else
    check_result(result, "         31", "%d in $swrite failed!");
`endif
    $swrite(result, "%d", $unsigned(31));
`ifdef OLD_UNSIZED
    check_result(result, "31", "%d in $swrite failed!");
`else
    check_result(result, "        31", "%d in $swrite failed!");
`endif
    $swrite(result, "%0d", val);
    check_result(result, "1000", "%0d in $swrite failed!");
    $swrite(result, "%+d", val);
    check_result(result, "      +1000", "%+d in $swrite failed!");
    $swrite(result, "%14d", val);
    check_result(result, "          1000", "%14d in $swrite failed!");
    $swrite(result, "%-14d", val);
    check_result(result, "1000          ", "%-14d in $swrite failed!");

    // %e
    $swrite(result, "%e", rval);
    check_result(result, "1.234567e+03", "%e in $swrite failed!");
    $swrite(result, "%E", rval);
    check_result(result, "1.234567E+03", "%E in $swrite failed!");
    $swrite(result, "%+e", rval);
    check_result(result, "+1.234567e+03", "%+e in $swrite failed!");
    $swrite(result, "%14.3e", rval);
    check_result(result, "     1.235e+03", "%14.3e in $swrite failed!");
    $swrite(result, "%-14.3e", rval);
    check_result(result, "1.235e+03     ", "%-14.3e in $swrite failed!");

    // %f
    $swrite(result, "%f", rval);
    check_result(result, "1234.567000", "%f in $swrite failed!");
    $swrite(result, "%F", rval);
    check_result(result, "1234.567000", "%F in $swrite failed!");
    $swrite(result, "%+f", rval);
    check_result(result, "+1234.567000", "%+f in $swrite failed!");
    $swrite(result, "%14.3f", rval);
    check_result(result, "      1234.567", "%14.3f in $swrite failed!");
    $swrite(result, "%-14.3f", rval);
    check_result(result, "1234.567      ", "%-14.3f in $swrite failed!");

    // %g
    $swrite(result, "%g", rval);
    check_result(result, "1234.57", "%g in $swrite failed!");
    $swrite(result, "%G", rval);
    check_result(result, "1234.57", "%G in $swrite failed!");
    $swrite(result, "%+g", rval);
    check_result(result, "+1234.57", "%+g in $swrite failed!");
    $swrite(result, "%14.3g", rval);
    check_result(result, "      1.23e+03", "%14.3g in $swrite failed!");
    $swrite(result, "%-14.3G", rval);
    check_result(result, "1.23E+03      ", "%-14.3G in $swrite failed!");

    // %l is currently unsupported.
    $swrite(result, "%l");
    check_result(result, "<%l>", "%l in $swrite failed!");
    $swrite(result, "%L");
    check_result(result, "<%L>", "%L in $swrite failed!");

    // %m
    $swrite(result, "%m");
    check_result(result, "top", "%m in $swrite failed!");
    $swrite(result, "%M");
    check_result(result, "top", "%M in $swrite failed!");
    $swrite(result, "%8m");
    check_result(result, "     top", "%m in $swrite failed!");
    $swrite(result, "%-8m");
    check_result(result, "top     ", "%m in $swrite failed!");

    // %s
    $swrite(result, "%s", "Hello");
    check_result(result, "Hello", "%s in $swrite failed!");
    $swrite(result, "%S", "Hello");
    check_result(result, "Hello", "%S in $swrite failed!");
    $swrite(result, str, "Hello");
    check_result(result, "Hello", "%s in $swrite failed!");
    $swrite(result, "%14s", "Hello");
    check_result(result, "         Hello", "%14s in $swrite failed!");
    $swrite(result, "%-14s", "Hello");
    check_result(result, "Hello         ", "%-14s in $swrite failed!");

    // %t
    $swrite(result, "%t", 0);
    check_result(result, "           0.0000 ps", "%t in $swrite failed!");
    $swrite(result, "%t", 1);
    check_result(result, "        1000.0000 ps", "%t in $swrite failed!");
    $swrite(result, "%T", 1);
    check_result(result, "        1000.0000 ps", "%T in $swrite failed!");
    $swrite(result, "%t", 10_000);
    check_result(result, "    10000000.0000 ps", "%t in $swrite failed!");
    $swrite(result, "%t", $time);
    check_result(result, "        1000.0000 ps", "%t $time in $swrite failed!");
//    $swrite(result, "%t", $simtime);
//    check_result(result, "        1000.0000 ps",
//                 "%t $simtime in $swrite failed!");
    $swrite(result, "%-t", 1);
    check_result(result, "1000.0000 ps        ", "%-t in $swrite failed!");
    $swrite(result, "%15t", 1);
    check_result(result, "   1000.0000 ps", "%15t in $swrite failed!");
    $swrite(result, "%-15t", 1);
    check_result(result, "1000.0000 ps   ", "%-15t in $swrite failed!");
    $swrite(result, "%15.1t", 1);
    check_result(result, "      1000.0 ps", "%15.1t in $swrite failed!");
    // Real values.
    $swrite(result, "%t", 1.1);
    check_result(result, "        1100.0000 ps", "%t in $swrite failed!");
    $swrite(result, "%t", $realtime);
    check_result(result, "        1000.0000 ps",
                 "%t $realtime in $swrite failed!");
    $swrite(result, "%-t", 1.1);
    check_result(result, "1100.0000 ps        ", "%-t in $swrite failed!");
    $swrite(result, "%15t", 1.1);
    check_result(result, "   1100.0000 ps", "%15t in $swrite failed!");
    $swrite(result, "%-15t", 1.1);
    check_result(result, "1100.0000 ps   ", "%-15t in $swrite failed!");
    $swrite(result, "%15.1t", 1.1);
    check_result(result, "      1100.0 ps", "%15.1t in $swrite failed!");

    // %u
    $swrite(result, "%u", eval);
    check_result(result, "\"", "%u in $swrite failed!");
    $swrite(result, "%U", eval);
    check_result(result, "\"", "%U in $swrite failed!");
    $swrite(result, "%u", sval);
    check_result(result, "Help me!", "%u in $swrite failed!");
    // "Help me!"
    $swrite(result, "%u", hval);
    check_result(result, "Help me!", "%u in $swrite failed!");
    // "Help" with check for correct x and z functionality.
    $swrite(result, "%u", uval);
    check_result(result, "Help", "%u in $swrite failed!");

    // %v
    $swrite(result, "%v", net);
    check_result(result, "Pu1", "%v in $swrite failed!");
    $swrite(result, "%V", net);
    check_result(result, "Pu1", "%V in $swrite failed!");
    $swrite(result, "%14v", net);
    check_result(result, "           Pu1", "%14v in $swrite failed!");
    $swrite(result, "%-14v", net);
    check_result(result, "Pu1           ", "%-14v in $swrite failed!");

    // %z
    $swrite(result, "%z", eval);
    check_result(result, "\"", "%z in $swrite failed!");
    $swrite(result, "%Z", eval);
    check_result(result, "\"", "%Z in $swrite failed!");
    // "Help me!", but because of NULLs we only get "Help"
    $swrite(result, "%z", hval);
    check_result(result, "Help", "%z in $swrite failed!");
    // "Help me!" encoded using all the states!
    $swrite(result, "%z", zval);
    check_result(result, "Help me!", "%z in $swrite failed!");

    // $sformat()
    $sformat(result, "%s", "Hello world");
    check_result(result, "Hello world", "String in $sformat failed!");
    $sformat(result, str, "Hello world");
    check_result(result, "Hello world", "Parameter in $sformat failed!");
    $sformat(result, fmt, "Hello world");
    check_result(result, "Hello world", "Register in $sformat failed!");

    $sformat(result, "%s");
    check_result(result, "<%s>", "$sformat missing argument failed!");
    $sformat(result, "%s", "Hello world", 2);
    check_result(result, "Hello world", "$sformat extra argument failed!");

    if (!failed) $display("All tests passed.");

  end
endmodule
