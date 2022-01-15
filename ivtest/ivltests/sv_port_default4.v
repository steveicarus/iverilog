
// This tests the basic support for default arguments to task/function
// ports. The default port syntax gives SystemVerilog a limited form
// of variable argument lists.

program main;

   class foo_t;
      int int_val;
      logic [3:0] log_val;

      task init (int int_init, logic[3:0] log_init = 4'bzzzz);
	 int_val = int_init;
	 log_val = log_init;
      endtask

   endclass : foo_t


   foo_t obj1;

   initial begin
      obj1 = new;
      obj1.init(5, 4'b1111);
      if (obj1.int_val != 5 || obj1.log_val !== 4'b1111) begin
	 $display("FAILED -- obj1.int_val=%0d, obj1.log_val=%b", obj1.int_val, obj1.log_val);
	 $finish;
      end

      obj1 = new;
      obj1.init(7);
      if (obj1.int_val != 7 || obj1.log_val !== 4'bzzzz) begin
	 $display("FAILED -- obj1.int_val=%0d, obj1.log_val=%0s", obj1.int_val, obj1.log_val);
	 $finish;
      end

      $display("PASSED");
   end

endprogram // main
