
class test_t;
   enum bit [1:0] { X, Y } foo;

   task go;
      foo = X;
      $display("test_t.foo=%b (X==0)", foo);
      if (foo !== X) begin
	 $display("FAILED");
	 $finish;
      end
      foo = Y;
      $display("test_t.foo=%b (Y==1)", foo);
      if (foo !== Y) begin
	 $display("FAILED");
	 $finish;
      end
   endtask
endclass // test_t

module main;
   typedef enum bit [1:0] { X, Y } xy_t;
   xy_t foo;

   initial begin
      foo = Y;
      $display("foo=%b (Y==1)", foo);
      if (foo !== Y) begin
	 $display("FAILED");
	 $finish;
      end
      foo = X;
      $display("foo=%b (X==0)", foo);
      if (foo !== X) begin
	 $display("FAILED");
	 $finish;
      end
   end

   test_t bar;
   initial begin
      bar = new;
      bar.go();
   end

   initial begin
      #1 $display("PASSED");
      $finish;
   end

endmodule // main
