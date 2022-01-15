
/*
 * This tests a trivial class. This tests that simple user defined
 * constructors work, and also tests shallow copoy "new".
 */
program main;

   // Trivial examples of classes.
   class foo_t ;
      int value;

      function new();
	 value = 42;
      endfunction // new

   endclass : foo_t // foo_t

   class bar_t ;
      int value;

      function new (int init);
	value = init;
      endfunction // new

   endclass : bar_t // foo_t

   foo_t obj1;
   bar_t obj2;
   foo_t obj1b;

   initial begin
      obj1 = new;
      if (obj1.value !== 42) begin
	 $display("FAILED -- Default constructor left value=%0d.", obj1.value);
	 $finish;
      end

      obj2 = new(53);
      if (obj2.value !== 53) begin
	 $display("FAILED -- new(53) constructure left value=%0d.", obj2.value);
	 $finish;
      end

      // Shallow object copy
      obj1b = new obj1;
      if (obj1b.value !== 42) begin
	 $display("FAILED -- Shallow copy constructor left value=%0d.", obj1b.value);
	 $finish;
      end

      obj1.value = 85;
      if (obj1b.value !== 42) begin
	 $display("FAILED -- Shallow copied value changed to %0d.", obj1b.value);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endprogram // main
