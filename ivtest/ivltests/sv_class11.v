
/*
 * This tests a trivial class. This tests that simple user defined
 * methods work.
 */
program main;

   // Trivial example of a class
   class foo_t ;
      int value_;

      task clear();
	 value_ = 0;
      endtask // clear

      task add(int x);
	 this.value_ = this.value_ + x;
      endtask // add

      function int peek();
	 peek = value_;
      endfunction // peek

      function int is_multiple(int x);
	 if (value_ % x == 0)
	   is_multiple = 1;
	 else
	   is_multiple = 0;
      endfunction // is_multiple

   endclass : foo_t // foo_t

   foo_t obj;

   initial begin
      obj = new;

      obj.clear();
      if (obj.peek() != 0) begin
	 $display("FAILED -- obj.value_=%0d after clear.", obj.value_);
	 $finish;
      end

      obj.add(5);
      if (obj.peek() != 5) begin
	 $display("FAILED -- obj.value_=%0d after add(5).", obj.value_);
	 $finish;
      end

      if (obj.is_multiple(2) != 0) begin
	 $display("FAILED -- obj.is_multipe(2) incorrect result. (5/2)");
	 $finish;
      end

      obj.add(3);
      if (obj.peek() != 8) begin
	 $display("FAILED -- obj.value_=%0d after add(3).", obj.value_);
	 $finish;
      end

      if (obj.is_multiple(2) == 0) begin
	 $display("FAILED -- obj.is_multipe(2) incorrect result. (8/2)");
	 $finish;
      end

      obj.clear();
      if (obj.peek() != 0) begin
	 $display("FAILED -- obj.value_=%0d after second clear.", obj.value_);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endprogram // main
