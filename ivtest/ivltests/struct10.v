module main;

   struct packed {
      bit [7:0][7:0] a;
      bit [15:0] b;
   } foo;

   struct packed {
      bit [0:7] [7:0] a;
      bit [0:15] b;
   } bar;

   initial begin
      foo = '0;
      foo.a[2:1] = 16'h1234;
      foo.a[5] = 8'h42;
      foo.a[7] = '1;
      foo.a[7][1:0] = '0;
      foo.b = '1;
      foo.b[1:0] = '0;

      $display("foo = %h", foo);

      bar = '0;
      bar.a[5:6] = 16'h1234;
      bar.a[2] = 8'h42;
      bar.a[0] = '1;
      bar.a[0][1:0] = '0;
      bar.b = '1;
      bar.b[14:15] = '0;

      $display("bar = %h", bar);

      if (foo !== 80'hFC00_4200_0012_3400_FFFC) begin
	 $display("FAILED -- foo=%h", foo);
	 $finish;
      end

      if (foo !== bar) begin
	 $display("FAILED -- bar != foo");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
