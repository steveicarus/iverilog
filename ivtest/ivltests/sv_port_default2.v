
// This tests the basic support for default arguments to task/function
// ports. The default port syntax gives SystemVerilog a limited form
// of variable argument lists.

program main;

   class foo_t;
      int int_val;
      string text_val;

      task init (int int_init, string text_init = "default text");
	 int_val = int_init;
	 text_val = text_init;
      endtask

   endclass : foo_t


   foo_t obj1;

   initial begin
      obj1 = new;
      obj1.init(5, "new text");
      if (obj1.int_val != 5 || obj1.text_val != "new text") begin
	 $display("FAILED -- obj1.int_val=%0d, obj1.text_val=%0s", obj1.int_val, obj1.text_val);
	 $finish;
      end

      obj1 = new;
      obj1.init(7);
      if (obj1.int_val != 7 || obj1.text_val != "default text") begin
	 $display("FAILED -- obj1.int_val=%0d, obj1.text_val=%0s", obj1.int_val, obj1.text_val);
	 $finish;
      end

      $display("PASSED");
   end

endprogram // main
