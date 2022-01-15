
/*
 * This tests a trivial class. This tests that properties can be
 * given types, and that the types behave properly.
 */
program main;

   class bar_t;
      int a;
      int b;
   endclass // bar_t

   // Trivial example of a class
   class foo_t ;
      byte a;
      bar_t b;
   endclass : foo_t // foo_t

   foo_t obj;
   bar_t tmp;

   initial begin
      obj = new;

      // This is the most trivial assignment of class properties.
      obj.a = 'hf_ff;
      obj.b = new;

      tmp = obj.b;
      tmp.a = 0;
      tmp.b = 1;

      if (obj.a != -1) begin
	 $display("FAILED -- assign to object: obj.a=%0d", obj.a);
	 $finish;
      end

      if (tmp.a != 0 || tmp.b != 1) begin
	 $display("FAILED -- obj.b.a=%0d, obj.b.b=%0d", tmp.a, tmp.b);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endprogram // main
