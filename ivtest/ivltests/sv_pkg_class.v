package p_defs;
   class a_class;
      int id_;

      function new(int id);
         id_ = id;
      endfunction

      task display();
         $display("This is class %0d.", id_);
      endtask

   endclass

endpackage

// This should print the following:
//    This is class 2.
//    This is class 1.
module top;
   import p_defs::a_class;

   a_class ac1;
   a_class ac2;
   initial begin
      ac1 = new(1);
      ac2 = new(2);
      ac2.display();
      ac1.display();
   end
endmodule
