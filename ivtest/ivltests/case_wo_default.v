
module test
  (output reg [1:0] foo,
   input wire foo_en1, foo_en2
   /* */);

   always @* begin
      foo = 0;
      case (1'b1)
	foo_en1 : foo = 1;
	foo_en2 : foo = 2;
      endcase
   end
endmodule // test

module main;
   wire [1:0] foo;
   reg	      foo_en1, foo_en2;

   test dut (.foo(foo), .foo_en1(foo_en1), .foo_en2(foo_en2));

   task fail;
      begin
	 $display("FAILED -- foo=%b, foo_en1=%b, foo_en2=%b",
		  foo, foo_en1, foo_en2);
	 $finish;
      end
   endtask // fail

   initial begin
      foo_en1 = 0;
      foo_en2 = 0;

      #1 if (foo !== 2'd0)
	fail;

      foo_en2 = 1;
      #1 if (foo !== 2'd2)
	fail;

      foo_en1 = 1;
      #1 if (foo !== 2'd1)
	fail;

      foo_en2 = 0;
      #1 if (foo !== 2'd1)
	fail;

      $display("PASSED");
   end // initial begin

endmodule // main
