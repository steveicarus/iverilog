// Passive check of every notifier change. Initial/final parity is insufficient.
`timescale 1ps/1ps
module period_history_check #(parameter COUNT=1, T0=0, T1=0)
                           (input wire notifier);
  integer seen = 0;
  reg armed = 0;
  initial begin
    #1;
    if (notifier !== 1'b0) $fatal(1, "FAIL initial notifier %m");
    armed = 1;
  end
  always @(notifier) if (armed) begin
    if (seen >= COUNT) $fatal(1, "FAIL extra notifier event %m at %0t", $time);
    if ((seen == 0 && $time != T0) || (seen == 1 && $time != T1))
      $fatal(1, "FAIL notifier time %m at %0t", $time);
    if (notifier !== ((seen & 1) ? 1'b0 : 1'b1))
      $fatal(1, "FAIL notifier value %m at %0t", $time);
    seen = seen + 1;
  end
  task complete;
    if (seen != COUNT) $fatal(1, "FAIL missing notifier events %m: %0d/%0d", seen, COUNT);
  endtask
endmodule
