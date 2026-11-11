// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/1ps

module hier_event_source #(parameter OFFSET = 0);
  reg state;
  initial begin
    state = 0;
    #(5 + OFFSET) state = 1;
    #3 state = 0;
    #3 state = 1;
    #3 state = 0;
  end
endmodule

module hier_event_pair #(parameter OFFSET = 0, A0 = 5, A1 = 8,
                         A2 = 11, A3 = 14, DONE = 15);
  hier_event_source #(OFFSET) source();
  integer any_count = 0, pos_count = 0, neg_count = 0;
  reg armed = 0;
  initial begin
    #1 armed = 1;
    #(DONE - 1);
    if (any_count != 4 || pos_count != 2 || neg_count != 2)
      $fatal(1, "FAIL counts %m %0d %0d %0d", any_count, pos_count, neg_count);
  end
  always @(source.state) if (armed) begin
    any_count = any_count + 1;
    case (any_count)
      1: if ($time != A0) $fatal(1, "FAIL any time 1 %m");
      2: if ($time != A1) $fatal(1, "FAIL any time 2 %m");
      3: if ($time != A2) $fatal(1, "FAIL any time 3 %m");
      4: if ($time != A3) $fatal(1, "FAIL any time 4 %m");
      default: $fatal(1, "FAIL extra any event %m");
    endcase
  end
  always @(posedge source.state) if (armed) begin
    pos_count = pos_count + 1;
    if ($time != (pos_count == 1 ? A0 : A2)) $fatal(1, "FAIL pos time %m");
  end
  always @(negedge source.state) if (armed) begin
    neg_count = neg_count + 1;
    if ($time != (neg_count == 1 ? A1 : A3)) $fatal(1, "FAIL neg time %m");
  end
endmodule

module hier_event_sibling;
  integer count = 0;
  reg armed = 0;
  initial begin
    #1 armed = 1;
    #14;
    if (count != 4) $fatal(1, "FAIL sibling count");
  end
  always @(tb.left.source.state) if (armed) begin
    count = count + 1;
    if ($time != 3 * count + 2) $fatal(1, "FAIL sibling time");
  end
endmodule

module tb;
  hier_event_pair left();
  hier_event_pair #(20, 25, 28, 31, 34, 35) renamed();
  hier_event_sibling sibling();
  reg local_tick = 0;
  integer mixed_count = 0, local_count = 0;
  reg armed = 0;
  initial begin
    #1 armed = 1;
    #2 local_tick = 1;
    #33;
    if (mixed_count != 3 || local_count != 1) $fatal(1, "FAIL mixed/local count");
    $display("PASSED");
    $finish;
  end
  always @(posedge renamed.source.state or posedge local_tick) if (armed) begin
    mixed_count = mixed_count + 1;
    case (mixed_count)
      1: if ($time != 3) $fatal(1, "FAIL mixed time 1");
      2: if ($time != 25) $fatal(1, "FAIL mixed time 2");
      3: if ($time != 31) $fatal(1, "FAIL mixed time 3");
      default: $fatal(1, "FAIL extra mixed event");
    endcase
  end
  always @(local_tick) if (armed) begin
    local_count = local_count + 1;
    if ($time != 3) $fatal(1, "FAIL local event time");
  end
endmodule
