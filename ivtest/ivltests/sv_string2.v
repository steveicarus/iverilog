/*
 * This is the most basic test of string variables.
 */
module main;

   string foo;
   string bar;

   initial begin
      foo = "foo";
      bar = "bar";

      if (foo != "foo") begin
	 $display("FAILED -- foo=%0s (1)", foo);
	 $finish;
      end

      if (bar != "bar") begin
	 $display("FAILED -- bar=%0s (2)", bar);
	 $finish;
      end

      if (foo == bar) begin
	 $display("FAILED -- %0s == %0s (3)", foo, bar);
	 $finish;
      end

      if (! (foo != bar)) begin
	 $display("FAILED -- ! (%0s != %0s) (4)", foo, bar);
	 $finish;
      end

      if (bar > foo) begin
	 $display("FAILED -- %s > %s (5)", bar, foo);
	 $finish;
      end

      if (bar >= foo) begin
	 $display("FAILED -- %s >= %s (6)", bar, foo);
	 $finish;
      end

      if (foo < bar) begin
	 $display("FAILED -- %s < %s (7)", foo, bar);
	 $finish;
      end

      if (foo <= bar) begin
	 $display("FAILED -- %s <= %s (8)", foo, bar);
	 $finish;
      end

      bar = foo;
      if (foo != bar) begin
	 $display("FAILED -- %0s != %0s (9)", foo, bar);
	 $finish;
      end

      if (foo > bar) begin
	 $display("FAILED -- %0s > %0s (10)", foo, bar);
	 $finish;
      end

      if (foo < bar) begin
	 $display("FAILED -- %0s < %0s (11)", foo, bar);
	 $finish;
      end

      if (! (foo == bar)) begin
	 $display("FAILED -- ! (%0s == %0s) (12)", foo, bar);
	 $finish;
      end

      if (! (foo <= bar)) begin
	 $display("FAILED -- ! (%0s <= %0s) (13)", foo, bar);
	 $finish;
      end

      if (! (foo >= bar)) begin
	 $display("FAILED -- ! (%0s >= %0s) (14)", foo, bar);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endmodule // main
