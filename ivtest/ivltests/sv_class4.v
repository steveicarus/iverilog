
/*
 * This tests a trivial class. This tests that properties can be
 * given types, and that the types behave properly.
 */
program main;

   // Trivial example of a class
   class foo_t ;
      int signed a;
      int unsigned b;
   endclass : foo_t // foo_t

   foo_t obj;

   initial begin
      obj = new;

      // This is the most trivial assignment of class properties.
      obj.a = 'hf_ffffffff;
      obj.b = 'hf_ffffffff;

      if (obj.a != -1 || obj.b != 'd4_294_967_295) begin
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
