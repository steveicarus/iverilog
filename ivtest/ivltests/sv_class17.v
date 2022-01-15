
/*
 * This tests a trivial class. This tests that simple property
 * initializers work.
 */
program main;

   // Trivial examples of classes.
   class foo_t ;
      int       int_value = 42;
      bit [3:0] bit_value = 5;
      string	txt_value = "text";
   endclass : foo_t // foo_t

   foo_t obj1;
   foo_t obj2;

   initial begin
      obj1 = new;

      // The shallow copy constructor bypasses (or at least overrides)
      // property declaration assignments, so obj2 should hold the
      // updated values and not the constructed values.
      obj1.int_value = 43;
      obj1.bit_value = 10;
      obj1.txt_value = "fluf";
      obj2 = new obj1;

      if (obj2.int_value !== 43) begin
	 $display("FAILED -- obj2.int_value=%0d.", obj2.int_value);
	 $finish;
      end

      if (obj2.bit_value !== 4'd10) begin
	 $display("FAILED -- obj2.bit_value=%0b.", obj2.bit_value);
	 $finish;
      end

      if (obj2.txt_value != "fluf") begin
	 $display("FAILED -- obj2.txt_value=%s", obj2.txt_value);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endprogram // main
