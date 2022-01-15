
module test(input wire load, in,
	    output reg out1, out2);

   (* ivl_combinational *)
   always @* begin
      out1 = 0;
      if (load) begin
	 out1 = in;
	 out2 = in;
      end else begin
	 out2 = ~in;
      end
   end

endmodule // test

module test_bench;

   reg load;
   reg val;
   wire out1, out2;

   test DUT(.load(load), .in(val), .out1(out1), .out2(out2));

   (* ivl_synthesis_off *)
   initial begin
      val = 0;
      load = 1;
      #1 ;
      if (out1 !== 0 || out2 !== 0) begin
	 $display("FAILED -- load=%b, val=%b, out1=%b, out2=%b", load, val, out1, out2);
	 $finish;
      end

      val = 1;
      #1 ;
      if (out1 !== 1 || out2 !== 1) begin
	 $display("FAILED -- load=%b, val=%b, out1=%b, out2=%b", load, val, out1, out2);
	 $finish;
      end

      load = 0;
      #1 ;
      if (out1 !== 0 || out2 !== 0) begin
	 $display("FAILED -- load=%b, val=%b, out1=%b, out2=%b", load, val, out1, out2);
	 $finish;
      end

      val = 0;
      #1 ;
      if (out1 !== 0 || out2 !== 1) begin
	 $display("FAILED -- load=%b, val=%b, out1=%b, out2=%b", load, val, out1, out2);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // test_bench
