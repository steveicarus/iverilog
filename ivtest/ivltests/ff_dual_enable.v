
module test
  (input wire clk,
   output reg foo, bar,
   input wire foo_valid, foo_in,
   input wire bar_valid, bar_in
   /* */);

   always @(posedge clk)
     begin
	if (foo_valid) foo <= foo_in;
	if (bar_valid) bar <= bar_in;
     end

endmodule // test


module main;
   reg clk;
   wire foo, bar;
   reg	foo_valid, foo_in;
   reg  bar_valid, bar_in;

   test dut (.clk(clk),
	     .foo(foo), .bar(bar),
	     .foo_valid(foo_valid), .bar_valid(bar_valid),
	     .foo_in(foo_in), .bar_in(bar_in));
   task fail;
      begin
	 $display("FAILED -- foo/bar=%b/%b, foo/bar_valid=%b/%b, foo/bar_in=%b/%b",
		  foo, bar, foo_valid, bar_valid, foo_in, bar_in);
	 $finish;
      end
   endtask // fail

   initial begin
      clk = 0;
      foo_valid = 1;
      bar_valid = 1;
      foo_in = 0;
      bar_in = 0;

      #1 clk = 1;
      #1 clk = 0;

      if (foo !== 0 || bar !== 0)
	fail;

      bar_in = 1;

      #1 clk = 1;
      #1 clk = 0;

      if (foo !== 0 || bar !== 1)
	fail;

      foo_in = 1;
      bar_in = 0;
      foo_valid = 1;
      bar_valid = 0;

      #1 clk = 1;
      #1 clk = 0;

      if (foo !== 1 || bar !== 1)
	fail;

      $display("PASSED");
   end

endmodule // main
