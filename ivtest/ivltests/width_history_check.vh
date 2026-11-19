// Expectations are supplied to a VPI observer active at cbStartOfSimulation.
`timescale 1ps/1ps
module width_history_check #(parameter COUNT=1, T0=0, T1=0, INITIAL=1'b0)
                           (input wire notifier);
  task complete;
    $pulse_history_complete(notifier);
  endtask
endmodule
