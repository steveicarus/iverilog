
// This tests SystemVerilog packages. Make sure that typedef
// names work.

package p1;
   localparam step = 5;
   task foo(output int y, input int x);
      y = x + step;
   endtask // foo
endpackage

module main;

   import p1::foo;
   int y, x;

   initial begin
      x = 5;
      foo(y, x);

      if (y != 10) begin
	 $display("FAILED -- x=%0d, y=%0d", x, y);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin
endmodule // main
