/*
 * Hierarchical event testcase
 *
 * Copyright (C) 2002 Charles Lepple <clepple@ghz.cc>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 * compilation options: none necessary
 */

`define TEST_HIERARCHICAL_EVENT
// yields parse error when defined

module top();
   event toplevel_event;

   submodule sub();

   initial #10 -> toplevel_event;
endmodule // top

module submodule();
   event local_event;

   initial #25 -> local_event;

   always
     begin
`ifdef TEST_HIERARCHICAL_EVENT
	@top.toplevel_event
	  $display("at %0d: toplevel event triggered", $time);
`endif
	@local_event
	  $display("at %0d: local event triggered", $time);
     end
endmodule // submodule

// local variables:
// verilog-simulator: "iverilog"
// end:
