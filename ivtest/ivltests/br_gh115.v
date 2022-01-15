`begin_keywords "1364-2005"
// Adapted from case6.v
module main;

   reg bit, foo;

   // Combinational device that sends 1 or 0 to foo, to follow bit.
   always @*
     begin
	foo = 1;
	case (bit)
	  1'b0: foo = 0;
	endcase // case(bit)
     end

   (* ivl_synthesis_off *)
   initial begin
      bit = 0;

      # 6 $display("bit=%b, foo=%b", bit, foo);
      if (bit !== 0 || foo !== 0) begin
	 $display("FAILED");
	 $finish;
      end

      bit <= 1;
      #10 $display("bit=%b, foo=%b", bit, foo);
      if (bit !== 1 || foo !== 1) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
`end_keywords
