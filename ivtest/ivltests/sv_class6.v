
/*
 * This tests a trivial class. This tests that properties can be
 * given types, and that the types behave properly.
 */
program main;

   // Trivial example of a class
   class foo_t ;
      real a;
      real b;
   endclass : foo_t // foo_t

   foo_t obj;

   initial begin
      obj = new;

      // This is the most trivial assignment of class properties.
      obj.a = 0.5;
      obj.b = -1.5;

      if (obj.a != 0.5 || obj.b != -1.5) begin
	 $display("FAILED -- assign to object: obj.a=%f, obj.b=%f", obj.a, obj.b);
	 $finish;
      end

      obj.a = obj.a - 0.5;
      obj.b = obj.b + 1.5;
      if (obj.a != 0.0 || obj.b != 0.0) begin
	 $display("FAILED -- increment properties: obj.a=%f, obj.b=%f", obj.a, obj.b);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endprogram // main
