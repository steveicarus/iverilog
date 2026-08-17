// A non-blocking event trigger may not target an automatic event.

module nb_event_trigger_auto_fail;

   task automatic trigger_event;
      event e;
      ->> e;
   endtask

   initial trigger_event();

endmodule
