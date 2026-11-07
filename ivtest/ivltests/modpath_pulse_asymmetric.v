// Check complete histories, not just final values, for asymmetric path delays.
// Default rejection uses the trailing transition's delay; equality survives.
`timescale 1ps/1ps

module asymmetric_path(input i, output o);
  assign o = i;
  specify
    (i => o) = (109, 82);
  endspecify
endmodule

module test;
  reg i, peer_i;
  wire o, peer_o;
  integer changes = 0;
  integer peer_changes = 0;
  reg failed = 0;

  asymmetric_path dut(i, o);
  asymmetric_path peer(peer_i, peer_o);

  task check;
    input time expected_time;
    input expected_value;
    begin
      if ($time != expected_time || o !== expected_value) begin
        $display("change %0d: got %0t/%b, expected %0t/%b",
                 changes, $time, o, expected_time, expected_value);
        failed = 1;
      end
    end
  endtask

  // Startup is checked separately; every change after readiness is scored.
  always @(o) if ($time >= 1000) begin
    case (changes)
       0: check(5109, 1);    // ordinary long high pulse
       1: check(5482, 0);
       2: check(7189, 1);    // rejected pair, then a fresh rise
       3: check(7582, 0);
       4: check(8109, 1);
       5: check(8662, 0);    // opposite-polarity rearm
       6: check(9109, 1);
       7: check(9582, 0);    // low pulse just beyond the reject boundary
       8: check(9692, 1);
       9: check(10082, 0);   // equality: leading callback and input return coincide
      10: check(10191, 1);
      11: check(10582, 0);
      12: check(11159, 1);   // negative nominal width, then rearm
      13: check(11582, 0);
      14: check(12269, 1);   // two rejected pairs, then rearm
      15: check(12582, 0);
      16: check(14109, 1);   // high pulse exactly at the reject boundary
      17: check(14191, 0);
      default: begin
        $display("extra change at %0t: %b", $time, o);
        failed = 1;
      end
    endcase
    changes = changes + 1;
  end

  // A second destination must keep its own value, due time and cancellation.
  always @(peer_o) if ($time >= 1000) begin
    if ((peer_changes == 0 && ($time != 7129 || peer_o !== 1'b1)) ||
        (peer_changes == 1 && ($time != 7502 || peer_o !== 1'b0)) ||
        peer_changes > 1) begin
      $display("unexpected peer change %0d at %0t: %b",
               peer_changes, $time, peer_o);
      failed = 1;
    end
    peer_changes = peer_changes + 1;
  end

  initial begin
    peer_i = 0;
    #7020 peer_i = 1;
    #400 peer_i = 0;
  end

  initial begin
    i = 0;
    #1000;
    if (o !== 1'b0 || peer_o !== 1'b0)
      $fatal(1, "initial outputs did not settle");

    #4000 i = 1;             // 5000: ordinary pulse
    #400 i = 0;
    #600 i = 1;              // 6000: short return, no output pulse
    #40 i = 0;
    #960 i = 1;              // 7000: short return and rearm
    #40 i = 0;
    #40 i = 1;
    #420 i = 0;

    #500 i = 1;              // 8000: prepare opposite polarity
    #500 i = 0;
    #40 i = 1;
    #40 i = 0;
    #420 i = 1;              // 9000
    #500 i = 0;
    #83 i = 1;               // valid low pulse: width 110 >= 109
    #417 i = 0;              // 10000
    #82 i = 1;               // equality low pulse: width 109
    #418 i = 0;              // 10500

    #500 i = 1;              // 11000: nominal fall precedes rise
    #20 i = 0;
    #30 i = 1;
    #450 i = 0;
    #500 i = 1;              // 12000: repeated cancellation
    #40 i = 0;
    #40 i = 1;
    #40 i = 0;
    #40 i = 1;
    #40 i = 1;               // duplicate assignment must not disturb delivery
    #300 i = 0;

    #500 i = 1;              // 13000: just below the boundary
    #108 i = 0;              // nominal width 81 < 82: reject both edges
    #892 i = 1;              // 14000
    #109 i = 0;              // nominal width 82 == 82: keep both edges
    #891;
    if (failed || changes != 18 || peer_changes != 2 ||
        o !== 1'b0 || peer_o !== 1'b0)
      $fatal(1, "asymmetric pulse history mismatch");
    $display("PASSED");
    $finish(0);
  end
endmodule
