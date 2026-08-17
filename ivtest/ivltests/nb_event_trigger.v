// Check the non-blocking event trigger (->>).
//
// The unobserved event is referenced only by a non-blocking trigger, so it
// must survive dangling-event removal. The observed event's immediate trigger
// must fire in the NBA region, and the delayed trigger issued at time 1 must
// fire at time 4.

module nb_event_trigger;

   event   e;
   event   unobserved;
   integer hits = 0;
   integer immediate_was_deferred = 0;
   time    last_hit;

   task automatic trigger_static_event;
      ->> e;
   endtask

   always @(e) begin
      hits = hits + 1;
      last_hit = $time;
   end

   initial begin
      ->> unobserved;
      trigger_static_event();
      immediate_was_deferred = (hits === 0);
      #1 ->> #3 e;
      #10;
      if (immediate_was_deferred && hits === 2 && last_hit === 4)
	$display("PASSED");
      else
	$display("FAILED -- deferred=%0d, expected 2 triggers with the last at time 4, got %0d trigger(s) with the last at time %0t",
		 immediate_was_deferred, hits, last_hit);
      $finish(0);
   end

endmodule
