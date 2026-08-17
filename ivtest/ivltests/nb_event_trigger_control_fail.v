// Event-controlled non-blocking triggers are parsed but not implemented.

module nb_event_trigger_control_fail;

   reg clk;
   event e;

   initial begin
      ->> @(posedge clk) e;
      ->> repeat (2) @(posedge clk) e;
   end

endmodule
