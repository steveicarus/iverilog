module top;
   event evt;
   reg rval;

   // Call user function with event (continuous assign).
   wire wval = func(evt);

   function func;
      input arg;
      begin
         $display("FAILED func.");
         func = 1'bx;
      end
   endfunction

   task tsk;
     input arg;
      begin
         $display("FAILED task.");
      end
   endtask

   // Call user function with event (procedural) and user task.
   initial begin
      rval = func(evt);
      tsk(evt);
   end
endmodule
