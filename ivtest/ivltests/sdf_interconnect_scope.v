// An INTERCONNECT delay belongs to the destination port's module in Icarus.
// Keep the SDF CELL, source, destination, top, and unrelated root independent.
// The oracle is physical time in ps; it does not read back annotated delays.
`ifndef TOP_UNIT
`define TOP_UNIT 1ns
`endif
`ifndef CELL_UNIT
`define CELL_UNIT 1ns
`endif
`ifndef CELL_PREC
`define CELL_PREC 1ps
`endif
`ifndef SOURCE_UNIT
`define SOURCE_UNIT 1ns
`endif
`ifndef LOAD_UNIT
`define LOAD_UNIT 1ns
`endif
`ifndef LOAD_PREC
`define LOAD_PREC 1ps
`endif
`ifndef TAIL_UNIT
`define TAIL_UNIT 1ns
`endif
`ifndef TAIL_PREC
`define TAIL_PREC 1ps
`endif
`ifndef TAIL_NAME
`define TAIL_NAME zz_unrelated
`endif
`ifndef EXPECT_RISE
`define EXPECT_RISE 7
`endif
`ifndef EXPECT_FALL
`define EXPECT_FALL 13
`endif

`timescale `SOURCE_UNIT/1ps
module interconnect_source(input a, output y);
  assign y = a;
  specify (a => y) = (0, 0); endspecify
endmodule

`timescale `LOAD_UNIT/`LOAD_PREC
module interconnect_load(input a, output y);
  assign y = a;
  specify (a => y) = (0, 0); endspecify
endmodule

`timescale `CELL_UNIT/`CELL_PREC
module interconnect_cell(input a, output y);
`ifdef UP_OUTPUT
  // port2 is an output of the containing module, not a child input.
  interconnect_source src(a, y);
`else
  wire link;
  interconnect_source src(a, link);
  interconnect_load dst(link, y);
`endif
endmodule

`timescale `TAIL_UNIT/`TAIL_PREC
module `TAIL_NAME;
  reg unused;
  initial unused = 0;
endmodule

`timescale `TOP_UNIT/1ps
module top;
  reg drive;
  wire result;
  integer events = 0;
  integer checks = 0;
  integer tick, expected_tick;
  reg expected_value;
  interconnect_cell dut(drive, result);

  initial begin
    $timeformat(-12, 0, " ps", 0);
`ifdef DIRECT_VPI
    if ($interconnect_scope_setup(dut.src, dut.dst, result) != 18)
      $fatal(1, "FAIL VPI setup");
`elsif UP_OUTPUT
    $sdf_annotate("ivltests/sdf_interconnect_scope_up.sdf", dut);
`else
    $sdf_annotate("ivltests/sdf_interconnect_scope.sdf", dut);
`endif
    drive = 0;
    // Even a mistaken 1000x scale leaves these transitions separate.
    #100ns drive = 1;
    #100ns drive = 0;
    #100ns drive = 1;
    #100ns drive = 0;
    #100ns;
    if (events != 4) $fatal(1, "FAIL missing events: %0d", events);
    if (checks != 8) $fatal(1, "FAIL assertion count: %0d", checks);
`ifdef DIRECT_VPI
    if ($interconnect_scope_finish() != 31)
      $fatal(1, "FAIL VPI callbacks or assertion count");
`endif
    $display("CHECKS %0d EVENTS %0d", checks, events);
    $display("PASSED");
    $finish(0);
  end

  // Ignore only the initial X-to-0 settling transition. All later output
  // events must match the complete four-event history, including its count.
  always @(result) if ($realtime >= 50ns) begin
    events = events + 1;
    tick = $realtime / 1ps;
    case (events)
      1: begin expected_tick = 100000 + `EXPECT_RISE; expected_value = 1; end
      2: begin expected_tick = 200000 + `EXPECT_FALL; expected_value = 0; end
      3: begin expected_tick = 300000 + `EXPECT_RISE; expected_value = 1; end
      4: begin expected_tick = 400000 + `EXPECT_FALL; expected_value = 0; end
      default: $fatal(1, "FAIL extra event at %0d ps", tick);
    endcase
    checks = checks + 1;
    if (result !== expected_value)
      $fatal(1, "FAIL event %0d value %b expected %b", events,
             result, expected_value);
    checks = checks + 1;
    if (tick !== expected_tick)
      $fatal(1, "FAIL event %0d at %0d ps expected %0d ps", events,
             tick, expected_tick);
    $display("EVENT %0d %b", tick, result);
  end
endmodule
