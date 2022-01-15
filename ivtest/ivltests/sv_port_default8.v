
// This tests the basic support for default arguments to task/function
// ports. The default port syntax gives SystemVerilog a limited form
// of variable argument lists.

program main;

   class foo_t;
      int int_val;
      logic[3:0] log_val;
      string text_val;

      task init (int int_init, logic[3:0]log_init = 4'bzzzz, string text_init = "default text");
	 this.init2(int_init, log_init, text_init);
      endtask : init

      function void init2 (int int_init, logic[3:0]log_init, string text_init);
	 int_val = int_init;
	 log_val = log_init;
	 text_val = text_init;
      endfunction : init2

   endclass : foo_t


   foo_t obj1;

   initial begin
      obj1 = new;
      obj1.init(4, 4'b0101, "new text");
      if (obj1.int_val != 4 || obj1.log_val !== 4'b0101 || obj1.text_val != "new text") begin
	 $display("FAILED -- obj1.int_val=%0d, obj1.log_val=%b obj1.text_val=%0s", obj1.int_val, obj1.log_val, obj1.text_val);
	 $finish;
      end

      obj1 = new;
      obj1.init(5, , "new text");
      if (obj1.int_val != 5 || obj1.log_val !== 4'bzzzz || obj1.text_val != "new text") begin
	 $display("FAILED -- obj1.int_val=%0d, obj1.log_val=%b obj1.text_val=%0s", obj1.int_val, obj1.log_val, obj1.text_val);
	 $finish;
      end

      obj1 = new;
      obj1.init(6, 4'b1010);
      if (obj1.int_val != 6 || obj1.log_val !== 4'b1010 || obj1.text_val != "default text") begin
	 $display("FAILED -- obj1.int_val=%0d, obj1.log_val=%b obj1.text_val=%0s", obj1.int_val, obj1.log_val, obj1.text_val);
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
