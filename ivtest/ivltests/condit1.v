`begin_keywords "1364-2005"
/*
 * This tests the synthesis of a case statement that has an empty case.
 */
module main;

   reg bit, foo, bar;

   // Combinational device that sends 1 or 0 to foo, to follow bit.
   // This tests the special situation that the if condition only sets
   // some of the bits that the process as a whole sets. This is OK if
   // the bits that are sometimes not set are covered elsewhere.
   always @*
     begin
	foo = 0;
	bar = 0;

	if (bit)
	  foo = 1;
	else
	  bar = 1;
     end

   (* ivl_synthesis_off *)
   initial begin
      bit = 0;

      # 6 $display("bit=%b, foo=%b, bar=%b", bit, foo, bar);
      if (bit !== 0 || foo !== 0 || bar !== 1) begin
	 $display("FAILED");
	 $finish;
      end

      bit <= 1;
      #10 $display("bit=%b, foo=%b, bar=%b", bit, foo, bar);
      if (bit !== 1 || foo !== 1 || bar !== 0) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
`end_keywords
