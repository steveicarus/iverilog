`timescale 1ps/1ps

// Authored UDPs: these are not PDK models. Unlisted combinations yield X.
primitive prepath_mux2_udp (q, d0, d1, sel);
  output q;
  input d0, d1, sel;
  table
    0 ? 0 : 0;
    1 ? 0 : 1;
    ? 0 1 : 0;
    ? 1 1 : 1;
    0 0 x : 0;
    1 1 x : 1;
  endtable
endprimitive

primitive prepath_mux4_udp (q, d0, d1, d2, d3, s0, s1);
  output q;
  input d0, d1, d2, d3, s0, s1;
  table
    0 ? ? ? 0 0 : 0;
    1 ? ? ? 0 0 : 1;
    ? 0 ? ? 1 0 : 0;
    ? 1 ? ? 1 0 : 1;
    ? ? 0 ? 0 1 : 0;
    ? ? 1 ? 0 1 : 1;
    ? ? ? 0 1 1 : 0;
    ? ? ? 1 1 1 : 1;
  endtable
endprimitive

module prepath_mux2 (output q, input d0, d1, sel);
  prepath_mux2_udp selected(q, d0, d1, sel);
  specify
    // Explicit twelve slots keep the X-edge oracle independent of expansion.
    (d0 => q) = (3,5,9,9,9,9,9,9,9,9,9,9);
    (d1 => q) = (3,5,9,9,9,9,9,9,9,9,9,9);
    (sel => q) = (3,5,9,9,9,9,9,9,9,9,9,9);
  endspecify
endmodule

module prepath_mux4 (output q, input d0, d1, d2, d3, s0, s1);
  prepath_mux4_udp selected(q, d0, d1, d2, d3, s0, s1);
  specify
    (d0 => q) = (7,11,13,13,13,13,13,13,13,13,13,13);
    (d1 => q) = (7,11,13,13,13,13,13,13,13,13,13,13);
    (d2 => q) = (7,11,13,13,13,13,13,13,13,13,13,13);
    (d3 => q) = (7,11,13,13,13,13,13,13,13,13,13,13);
    (s0 => q) = (7,11,13,13,13,13,13,13,13,13,13,13);
    (s1 => q) = (7,11,13,13,13,13,13,13,13,13,13,13);
  endspecify
endmodule

primitive prepath_sequential_udp(q, clk, d);
  output q;
  reg q;
  input clk, d;
  table
    r 0 : ? : 0;
    r 1 : ? : 1;
    f ? : ? : -;
    ? * : ? : -;
  endtable
endprimitive

module prepath_excluded_sequential(output q, input clk, d);
  prepath_sequential_udp seq(q, clk, d);
  specify (clk => q) = (2,2); endspecify
endmodule

module prepath_excluded_delayed(output q, input d0, d1, sel);
  prepath_mux2_udp #2 selected(q, d0, d1, sel);
  specify (sel => q) = (2,2); endspecify
endmodule

module prepath_excluded_assign(output q, input d);
  assign q = d;
  specify (d => q) = (2,2); endspecify
endmodule

module prepath_excluded_no_path(output q, input d0, d1, sel);
  prepath_mux2_udp selected(q, d0, d1, sel);
endmodule

module prepath_excluded_multiple(output q, input d0, d1, sel);
  prepath_mux2_udp selected_one(q, d0, d1, sel);
  prepath_mux2_udp selected_two(q, d1, d0, sel);
  specify (sel => q) = (2,2); endspecify
endmodule

module test;
`ifdef PREPATH_DISABLED
  parameter expect_enable = 0;
`else
  parameter expect_enable = 1;
`endif
  reg d0, d1, d2, d3, s0, s1;
  wire q2, q4, seq_q, delayed_q, assign_q, no_path_q, multiple_q;
`ifdef PREPATH_RENAMED
  prepath_mux4 alternate_large(q4, d0, d1, d2, d3, s0, s1);
  prepath_mux2 alternate_small(q2, d0, d1, s0);
`else
  prepath_mux2 first(q2, d0, d1, s0);
  prepath_mux4 second(q4, d0, d1, d2, d3, s0, s1);
`endif
  prepath_excluded_sequential excluded_seq(seq_q, s0, d0);
  prepath_excluded_delayed excluded_delay(delayed_q, d0, d1, s0);
  prepath_excluded_assign excluded_assign(assign_q, d0);
  prepath_excluded_no_path excluded_no_path(no_path_q, d0, d1, s0);
  prepath_excluded_multiple excluded_multiple(multiple_q, d0, d1, s0);

  initial begin
    // mux2 selects D0/D1; mux4 selects the distinct D2/D3 bank.
    d0 = 0; d1 = 1; d2 = 1; d3 = 0; s0 = 0; s1 = 1;
    #100 s0 = 1;
    #100 s0 = 0;
    #100 d3 = 1'bx; // Unselected data must not change either output.
    #50 d2 = 0;
    #50 d0 = 1'bx;
    #100 d0 = 1;
    #100 d1 = 1'bx; // Another unselected X.
    #100 s0 = 1;
    #100 s0 = 0;
    #100 d0 = 0;
    #100; // Natural end; avoid filename-dependent $finish diagnostics.
  end
endmodule
