// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/1ps

module hier_input_source #(parameter OFFSET = 0);
  reg state;
  reg declared = 0;
  reg \state.with.dot ;
  reg [7:4] word;
  initial begin
    state = 0;
    \state.with.dot = 0;
    word = 4'ha;
    #(5 + OFFSET);
    state = 1;
    declared = 1;
    \state.with.dot = 1;
    word = 4'h5;
    #4;
    state = 0;
    declared = 0;
    \state.with.dot = 0;
    word = 4'h3;
  end
endmodule

module hier_input_check #(parameter MID = 6, END = 10)
                       (input wire value, declared, escaped,
                        input wire [3:0] word);
  initial begin
    #1;
    if ({value, declared, escaped, word} !== 7'b0001010)
      $fatal(1, "FAIL initial inputs %m %b", {value, declared, escaped, word});
    #(MID - 1);
    if ({value, declared, escaped, word} !== 7'b1110101)
      $fatal(1, "FAIL changed inputs %m");
    #(END - MID);
    if ({value, declared, escaped, word} !== 7'b0000011)
      $fatal(1, "FAIL restored inputs %m");
  end
endmodule

module hier_input_pair #(parameter OFFSET = 0, MID = 6, END = 10);
  hier_input_source #(OFFSET) source();
  hier_input_check #(MID, END) sink(source.state, source.declared,
                        source.\state.with.dot , source.word);
endmodule

module tb;
  hier_input_pair left();
  hier_input_pair #(20, 26, 30) renamed();
  // Also preserve ordinary locally driven input actuals.
  reg local_value = 0;
  reg [3:0] local_word = 4'ha;
  hier_input_check local_sink(local_value, local_value, local_value, local_word);
  initial begin
    #5; local_value = 1; local_word = 4'h5;
    #4; local_value = 0; local_word = 4'h3;
    #22;
    $display("PASSED");
    $finish;
  end
endmodule
