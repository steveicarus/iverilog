
/*
 * This tests a trivial class. This tests that simple user defined
 * constructors work, and also tests shallow copoy "new".
 */
program main;

   class share_t;
      int value;
   endclass // share_t

   // Trivial examples of classes.
   class foo_t ;
      int value;
      string text;
      share_t common;
   endclass : foo_t // foo_t

   foo_t obj1;
   foo_t obj2;
   share_t tmp;

   initial begin
      obj1 = new;
      obj1.common = new;
      obj1.value = 42;
      obj1.text = "text";
      tmp = obj1.common;
      tmp.value = 54;

      if (obj1.value !== 42) begin
	 $display("FAILED -- obj1.value=%0d.", obj1.value);
	 $finish;
      end

      if (obj1.text != "text") begin
	 $display("FAILED -- obj1.text=%s", obj1.text);
	 $finish;
      end

      tmp = obj1.common;
      if (tmp.value !== 54) begin
	 $display("FAILED -- obj1.common.value=%0d.", tmp.value);
	 $finish;
      end

      obj2 = new obj1;
      obj1.value = 43;
      obj1.text = "new text";
      tmp = obj1.common;
      tmp.value = 53;

      if (obj1.value !== 43) begin
	 $display("FAILED -- obj1.value=%0d.", obj1.value);
	 $finish;
      end

      if (obj2.value !== 42) begin
	 $display("FAILED -- obj2.value=%0d.", obj2.value);
	 $finish;
      end

      if (obj1.text != "new text") begin
	 $display("FAILED -- obj1.text=%s", obj1.text);
	 $finish;
      end

      if (obj2.text != "text") begin
	 $display("FAILED -- obj2.text=%s", obj2.text);
	 $finish;
      end

      tmp = obj1.common;
      if (tmp.value !== 53) begin
	 $display("FAILED -- obj1.common.value=%0d.", tmp.value);
	 $finish;
      end

      tmp = obj2.common;
      if (tmp.value !== 53) begin
	 $display("FAILED -- obj2.common.value=%0d.", tmp.value);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endprogram // main
