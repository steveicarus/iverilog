class example;
   int id_;

   function new(int id);
      id_ = id;
   endfunction

   task display(int run);
      $display("  This is run %0d of class %0d", run, id_);
   endtask

   function int get_id();
      get_id = id_;
   endfunction

   task work();
      /* These method calls work correctly. */
      $display("Starting work with class %0d", this.get_id());
      this.display(1);
      /* Elaboration of these method calls fail. It looks like they are
       * being elaborated as a normal user functions/tasks not as a method
       * calls. */
      display(2);
      $display("Done with work for class %0d", get_id());
   endtask
endclass

module top;
   example test;
   initial begin
      test = new(1);
      $display("Created a class with id: %0d", test.get_id());
      test.work();
      $display("PASSED");
   end
endmodule
