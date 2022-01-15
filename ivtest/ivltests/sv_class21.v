
program main;

   class base_t ;
      int int_value;
      function new(int val);
	 int_value = val;
      endfunction // new
   endclass : base_t

   class foo_t extends base_t ;
      string str_value;
      function new();
	 super.new(42);
	 str_value = "42";
      endfunction
   endclass : foo_t

   foo_t obj1;

   initial begin
      obj1 = new;

      if (obj1.int_value !== 42) begin
	 $display("FAILED -- obj1.int_value = %0d", obj1.int_value);
	 $finish;
      end

      if (obj1.str_value != "42") begin
	 $display("FAILED -- obj1.str_value = %0s", obj1.str_value);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endprogram // main
