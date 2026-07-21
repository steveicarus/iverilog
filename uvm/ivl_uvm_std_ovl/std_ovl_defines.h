// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`ifdef OVL_STD_DEFINES_H
// do nothing
`else
`define OVL_STD_DEFINES_H

`define OVL_VERSION "V2.8"

`ifdef OVL_ASSERT_ON
  `ifdef OVL_PSL
     `ifdef OVL_VERILOG
        `undef OVL_PSL
     `endif 
     `ifdef OVL_SVA
        `ifdef OVL_PSL
          `undef OVL_PSL
        `endif
     `endif
  `else
    `ifdef OVL_VERILOG
    `else
      `define OVL_VERILOG
    `endif 
    `ifdef OVL_SVA
       `undef OVL_VERILOG
    `endif
  `endif
`endif

`ifdef OVL_COVER_ON
  `ifdef OVL_PSL
     `ifdef OVL_VERILOG
        `undef OVL_PSL
     `endif
     `ifdef OVL_SVA
        `ifdef OVL_PSL
          `undef OVL_PSL
        `endif
     `endif
  `else
    `ifdef OVL_VERILOG
    `else
      `define OVL_VERILOG
    `endif
    `ifdef OVL_SVA
       `undef OVL_VERILOG
    `endif
  `endif
`endif

`ifdef OVL_ASSERT_ON
  `ifdef OVL_SHARED_CODE
  `else
    `define OVL_SHARED_CODE
  `endif
`else
  `ifdef OVL_COVER_ON
    `ifdef OVL_SHARED_CODE
    `else
      `define OVL_SHARED_CODE
    `endif
  `endif
`endif

// specifying interface for System Verilog
`ifdef OVL_SVA_INTERFACE
  `define module interface
  `define endmodule endinterface
`else
  `define module module
  `define endmodule endmodule
`endif

// Selecting global reset or local reset for the checker reset signal
`ifdef OVL_GLOBAL_RESET
  `define OVL_RESET_SIGNAL `OVL_GLOBAL_RESET
`else
  `define OVL_RESET_SIGNAL reset_n
`endif

// active edges
`define OVL_NOEDGE  0
`define OVL_POSEDGE 1
`define OVL_NEGEDGE 2
`define OVL_ANYEDGE 3

// default edge_type (ovl_always_on_edge)
`ifdef OVL_EDGE_TYPE_DEFAULT
  // do nothing
`else
  `define OVL_EDGE_TYPE_DEFAULT `OVL_NOEDGE
`endif


// severity levels
`define OVL_FATAL   0
`define OVL_ERROR   1
`define OVL_WARNING 2
`define OVL_INFO    3

// default severity level
`ifdef OVL_SEVERITY_DEFAULT
  // do nothing
`else
  `define OVL_SEVERITY_DEFAULT `OVL_ERROR
`endif

// coverage levels (note that 3 would set both SANITY & BASIC)
`define OVL_COVER_NONE      0
`define OVL_COVER_SANITY    1
`define OVL_COVER_BASIC     2
`define OVL_COVER_CORNER    4
`define OVL_COVER_STATISTIC 8
`define OVL_COVER_ALL       15

// default coverage level
`ifdef OVL_COVER_DEFAULT
  // do nothing
`else
  `define OVL_COVER_DEFAULT `OVL_COVER_BASIC
`endif

// property type
`define OVL_ASSERT        0
`define OVL_ASSUME        1
`define OVL_IGNORE        2
`define OVL_ASSERT_2STATE 3
`define OVL_ASSUME_2STATE 4

// fire bit positions (first two also used for xcheck input to error_t)
`define OVL_FIRE_2STATE 0
`define OVL_FIRE_XCHECK 1
`define OVL_FIRE_COVER  2

// auto_bin_max for covergroups, default value is set as per LRM recommendation
`define OVL_AUTO_BIN_MAX_DEFAULT 64
`define OVL_AUTO_BIN_MAX `OVL_AUTO_BIN_MAX_DEFAULT 

// default property type
`ifdef OVL_PROPERTY_DEFAULT
  // do nothing
`else
  `define OVL_PROPERTY_DEFAULT `OVL_ASSERT
`endif

// default message
`ifdef OVL_MSG_DEFAULT
  // do nothing
`else
  `define OVL_MSG_DEFAULT "VIOLATION"
`endif

// necessary condition
`define OVL_TRIGGER_ON_MOST_PIPE    0
`define OVL_TRIGGER_ON_FIRST_PIPE   1
`define OVL_TRIGGER_ON_FIRST_NOPIPE 2

// default necessary_condition (ovl_cycle_sequence)
`ifdef OVL_NECESSARY_CONDITION_DEFAULT
  // do nothing
`else
  `define OVL_NECESSARY_CONDITION_DEFAULT `OVL_TRIGGER_ON_MOST_PIPE
`endif

// action on new start
`define OVL_IGNORE_NEW_START   0
`define OVL_RESET_ON_NEW_START 1
`define OVL_ERROR_ON_NEW_START 2

// default action_on_new_start (e.g. ovl_change)
`ifdef OVL_ACTION_ON_NEW_START_DEFAULT
  // do nothing
`else
  `define OVL_ACTION_ON_NEW_START_DEFAULT `OVL_IGNORE_NEW_START
`endif

// inactive levels
`define OVL_ALL_ZEROS 0
`define OVL_ALL_ONES  1
`define OVL_ONE_COLD  2

// default inactive (ovl_one_cold)
`ifdef OVL_INACTIVE_DEFAULT
  // do nothing
`else
  `define OVL_INACTIVE_DEFAULT `OVL_ONE_COLD
`endif

// new interface (ovl 2)
`define OVL_ACTIVE_LOW  0
`define OVL_ACTIVE_HIGH 1

`define OVL_GATE_NONE  0
`define OVL_GATE_CLOCK 1
`define OVL_GATE_RESET 2

`define OVL_FIRE_WIDTH   3

`ifdef OVL_CLOCK_EDGE_DEFAULT
  // do nothing
`else
  `define OVL_CLOCK_EDGE_DEFAULT `OVL_POSEDGE
`endif

`ifdef OVL_RESET_POLARITY_DEFAULT
  // do nothing
`else
`define OVL_RESET_POLARITY_DEFAULT `OVL_ACTIVE_LOW
`endif

`ifdef OVL_GATING_TYPE_DEFAULT
  // do nothing
`else
`define OVL_GATING_TYPE_DEFAULT `OVL_GATE_CLOCK
`endif

// ovl runtime after fatal error
`ifdef OVL_RUNTIME_AFTER_FATAL
  // do nothing
`else
`define OVL_RUNTIME_AFTER_FATAL 100
`endif

// Covergroup define
`ifdef OVL_COVER_ON
  `ifdef OVL_COVERGROUP_OFF
  `else
    `define OVL_COVERGROUP_ON
  `endif // OVL_COVERGROUP_OFF
`endif // OVL_COVER_ON

// Ensure x-checking logic disabled if ASSERTs are off
`ifdef OVL_ASSERT_ON
`else
  `define OVL_XCHECK_OFF
  `define OVL_IMPLICIT_XCHECK_OFF
`endif

`endif // OVL_STD_DEFINES_H
