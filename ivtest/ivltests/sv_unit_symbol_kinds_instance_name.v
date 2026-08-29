// Check that enclosing instance names do not affect compilation-unit lookup
// for parameters, named events, and imported variables.

package p;
  integer imported_value = 53;
endpackage

import p::*;

parameter integer parameter_value = 59;

event event_value;

module holder;
endmodule

module child;

  reg event_seen;
  reg failed;

  initial begin
    event_seen = 1'b0;
    failed = 1'b0;

    fork
      begin
        @event_value;
        event_seen = 1'b1;
      end
      begin
        #1 -> event_value;
      end
    join

    if (imported_value !== 53 || parameter_value !== 59 || !event_seen) begin
      failed = 1'b1;
    end

    if (failed) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule

module test;

  holder event_value();
  holder imported_value();
  holder parameter_value();
  child child_instance();

endmodule
