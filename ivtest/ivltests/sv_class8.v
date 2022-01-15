
/*
 * This tests a trivial class. This tests that properties can be
 * given types, and that the types behave properly.
 */
program main;

   // Trivial example of a class
   class foo_t ;
      byte a;
      int  b;
      real c;
      string d;
   endclass : foo_t // foo_t

   foo_t obj;

   initial begin
      obj = new;

      // This is the most trivial assignment of class properties.
      obj.a = 'hf_ff;
      obj.b = 'hf_ffffffff;
      obj.c = -1.5;
      obj.d = "-1";

      if (obj.a != -1 || obj.b != -1 || obj.c != -1.5 || obj.d != "-1") begin
	 $display("FAILED -- assign to object: obj.a=%0d, obj.b=%0d, obj.c=%f, obj.d=%0s", obj.a, obj.b, obj.c, obj.d);
	 $finish;
      end

      obj.a = obj.a + 1;
      obj.b = obj.b + 1;
      obj.c = obj.c + 1.5;
      if (obj.a != 0 || obj.b != 0 || obj.c != 0.0) begin
	 $display("FAILED -- increment properties: obj.a=%0d, obj.b=%0d, obj.c=%f", obj.a, obj.b, obj.c);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endprogram // main
