`timescale 1ns/10ps

module top;
  reg pass;
  real rise, fall, delay, base, diff;
  reg in, ctl;
  wire out, outif0;

  buf #(rise, fall) dut(out, in);
  bufif0 #(rise, fall) dutif0(outif0, in, ctl);

  // Check that the buffer output has the correct value and changed at the
  // correct time.
  always @(out) begin
    if ((in === 1'bz && out !== 1'bx) ||
        (in !== 1'bz && out !== in)) begin
      $display("in (%b) !== out (%b) at %.1f", in, out, $realtime);
      pass = 1'b0;
    end
    diff = $realtime - (base + delay);
    if (diff < 0.0) diff = -diff;
    if (diff >= 0.01) begin
      $display("Incorrect buf delay at %.1f, got %.1f, expected %.1f",
               base, $realtime-base, delay);
      pass = 1'b0;
    end
  end

  // Check that the bufif0 output has the correct value and changed at the
  // correct time.
  always @(outif0) begin
    if (ctl) begin
      if (outif0 !== 1'bz) begin
        $display("outif0 (%b) !== 1'bz at %.1f", out, $realtime);
        pass = 1'b0;
      end
    end else if ((in === 1'bz && outif0 !== 1'bx) ||
             (in !== 1'bz && outif0 !== in)) begin
      $display("in (%b) !== outif0 (%b) at %.1f", in, outif0, $realtime);
      pass = 1'b0;
    end
    diff = $realtime - (base + delay);
    if (diff < 0.0) diff = -diff;
    if (diff >= 0.01) begin
      $display("Incorrect bufif0 delay at %.1f, got %.1f, expected %.1f",
               base, $realtime-base, delay);
      pass = 1'b0;
    end
  end

  function real min(input real a, input real b);
    if (a < b) min = a;
    else min = b;
  endfunction

  initial begin
//    $monitor($realtime,,out,outif0,, in,, ctl);
    pass = 1'b1;
    rise = 1.1;
    fall = 1.2;
    ctl = 1'b0;
    // x -> 0 (fall)
    in = 1'b0;
    delay = fall;
    base = $realtime;
    #2;
    // 0 -> 1 (rise)
    in = 1'b1;
    delay = rise;
    base = $realtime;
    #2;
    // 1 -> x (min(rise, fall))
    delay = min(rise, fall);
    in = 1'bz;
    base = $realtime;
    #2;
    // x -> 1 (rise)
    in = 1'b1;
    delay = rise;
    base = $realtime;
    #2;
    // 1 -> 0 (fall)
    in = 1'b0;
    delay = fall;
    base = $realtime;
    #2;
    fall = 1.0;
    // 0 -> x (min(rise, fall))
    in = 1'bx;
    delay = min(rise, fall);
    base = $realtime;
    #2;
    // x -> z (min(rise, fall))
    ctl = 1'b1;
    delay = min(rise, fall);
    base = $realtime;
    #2;
    // z -> x (min(rise, fall))
    ctl = 1'b0;
    delay = min(rise, fall);
    base = $realtime;
    #2;
    fall = 1.2;
    // x -> z (min(rise, fall))
    ctl = 1'b1;
    delay = min(rise, fall);
    base = $realtime;
    #2;
    if (pass) $display("PASSED");
  end
endmodule
