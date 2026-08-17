// A named-event wait driven by a non-blocking trigger is not asynchronous
// logic and must not be rewritten by the synthesis pass.

module nb_event_trigger_synth;

   event e;
   reg   q;
   integer q_was_unknown;

   always @(e)
     q = 1'b1;

   initial begin
      q_was_unknown = (q === 1'bx);
      ->> e;
      #1;
      if (q_was_unknown && q === 1'b1)
	$display("PASSED");
      else
	$display("FAILED -- named-event wait was synthesized away (initial x=%0d, final q=%b)",
		 q_was_unknown, q);
      $finish(0);
   end

endmodule
