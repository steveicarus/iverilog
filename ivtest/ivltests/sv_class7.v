
/*
 * This tests a trivial class. This tests that properties can be
 * given types, and that the types behave properly.
 */
program main;

   // Trivial example of a class
   class foo_t ;
      string a;
      string b;
   endclass : foo_t // foo_t

   foo_t obj;

   initial begin
      obj = new;

      // Absent any other constructor, strings get initialized as nil.
      if (obj.a != "" || obj.b != "") begin
	 $display("FAILED -- String property not initialized.");
	 $finish;
      end

      // This is the most trivial assignment of class properties.
      obj.a = "Hello";
      obj.b = "World";

      $display("obj = {%0s, %0s}", obj.a, obj.b);
      if (obj.a != "Hello" || obj.b != "World") begin
	 $display("FAILED -- assign to object: obj.a=%0s, obj.b=%0s", obj.a, obj.b);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endprogram // main
