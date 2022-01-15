
module test(input wire load, drain,
	    input wire clk, data,
	    output reg foo_nxt, bar_nxt);

   reg		       foo, bar;

   (* ivl_combinational *)
   always @* begin
      foo_nxt = foo;
      bar_nxt = bar;

      if (load) begin
	 foo_nxt = data;
	 bar_nxt = 1;
      end else if (drain) begin
	 bar_nxt = 0;
      end
   end

   always @(posedge clk) begin
      foo <= foo_nxt;
      bar <= bar_nxt;
   end

endmodule // test

module main;

   reg clk, load, drain, data;
   wire foo, bar;

   test dut (.clk(clk), .load(load), .drain(drain), .data(data),
	     .foo_nxt(foo), .bar_nxt(bar));

   (* ivl_synthesis_off *)
   initial begin
      clk = 0;
      load = 1;
      drain = 0;
      data = 1;
      #1 clk = 1;
      #1 clk = 0;
      $display("%0t: load=%b, drain=%b, data=%b: foo=%b, bar=%b",
	       $time, load, drain, data, foo, bar);
      if (foo !== 1 || bar !== 1) begin
	 $display("FAILED -- foo=%b, bar=%b (1)", foo, bar);
	 $finish;
      end
      data = 0;
      #1 clk = 1;
      #1 clk = 0;
      $display("%0t: load=%b, drain=%b, data=%b: foo=%b, bar=%b",
	       $time, load, drain, data, foo, bar);
      if (foo !== 0 || bar !== 1) begin
	 $display("FAILED -- foo=%b, bar=%b (2)", foo, bar);
	 $finish;
      end
      load = 0;
      drain = 1;
      #1 ;
      if (foo !== 0 || bar !== 0) begin
	 $display("FAILED -- foo=%b, bar=%b (3)", foo, bar);
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;
      $display("%0t: load=%b, drain=%b, data=%b: foo=%b, bar=%b",
	       $time, load, drain, data, foo, bar);

      $display("PASSED");
   end

endmodule // main
