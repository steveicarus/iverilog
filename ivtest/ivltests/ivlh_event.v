module main;

   reg a, b;

   always @(a or b) begin
      if ($ivlh_attribute_event(a))
	$display("%0t: EVENT on a", $time);
      if ($ivlh_attribute_event(b))
	$display("%0t: EVENT on b", $time);
   end

   initial begin
      #1 a <= 1;
      #1 b <= 1;
      #1 a <= 0;
      #1 b <= 0;
      #1 $finish(0);
   end

endmodule // main
