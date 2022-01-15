
// This tests SystemVerilog packages. Make sure that names that
// are the same is different packages can be references properly
// in the various packages.

package p1;
   localparam step = 1;
   function int next_step(int x);
      next_step = x + step;
   endfunction // next_step
endpackage // p1

package p2;
   localparam step = 2;
   function int next_step(int x);
      next_step = x + step;
   endfunction // next_step
endpackage // p2

program main;
   int x;
   initial begin
      if (p1::step != 1) begin
	 $display("FAILED -- p1::step == %0d", p1::step);
	 $finish;
      end

      if (p2::step != 2) begin
	 $display("FAILED -- p2::step == %0d", p1::step);
	 $finish;
      end

      x = p1::next_step(0);
      if (x != 1) begin
	 $display("FAILED -- p1::next_step(0) --> %0d", x);
	 $finish;
      end

      x = p2::next_step(0);
      if (x != 2) begin
	 $display("FAILED -- p2::next_step(0) --> %0d", x);
	 $finish;
      end

      $display("PASSED");
   end
endprogram // main
