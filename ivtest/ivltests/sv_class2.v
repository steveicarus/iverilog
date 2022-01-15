
/*
 * This tests a trivial class. This tests that properties can be
 * given types, and that the types behave properly.
 */
program main;

   // Trivial example of a class
   class foo_t ;
      byte signed a;
      byte unsigned b;
   endclass : foo_t // foo_t

   foo_t obj;

   initial begin
      obj = new;

      // This is the most trivial assignment of class properties.
      obj.a = 'hfff;
      obj.b = 'hfff;

      if (obj.a != -1 || obj.b != 255) begin
	 $display("FAILED -- assign to object: obj.a=%0d, obj.b=%0d", obj.a, obj.b);
	 $finish;
      end

      obj.a = obj.a + 1;
      obj.b = obj.b + 1;
      if (obj.a != 0 || obj.b != 0) begin
	 $display("FAILED -- increment properties: obj.a=%0d, obj.b=%0d", obj.a, obj.b);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endprogram // main
