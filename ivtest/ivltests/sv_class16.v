
/*
 * This tests a trivial class. This tests that simple property
 * initializers work, but are overridden by an constructor.
 */
program main;

   // Trivial examples of classes.
   class foo_t ;
      int       int_value = 42;
      bit [3:0] bit_value = 5;
      string	txt_value = "text";

      function new();
	 // The declaration assignments happen before the constructor
	 // is called, so we can refer to them.
	 int_value = int_value + 1; // s.b. 43
	 bit_value = bit_value * 2; // s.b. 10
	 txt_value = "fluf";
      endfunction
   endclass : foo_t // foo_t

   foo_t obj1;

   initial begin
      obj1 = new;

      if (obj1.int_value !== 43) begin
	 $display("FAILED -- obj1.int_value=%0d.", obj1.int_value);
	 $finish;
      end

      if (obj1.bit_value !== 4'd10) begin
	 $display("FAILED -- obj1.bit_value=%0b.", obj1.bit_value);
	 $finish;
      end

      if (obj1.txt_value != "fluf") begin
	 $display("FAILED -- obj1.txt_value=%s", obj1.txt_value);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endprogram // main
