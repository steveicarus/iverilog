
module main;

   typedef enum { RED, ORANGE, YELLOW, GREEN, BLUE, VIOLET } rainbow_t;
   rainbow_t    color;
   string	tmp;

   initial begin
      color = color.first();
      tmp = color.name();
      $display("first color is %s (%0d)", tmp, color);
      if (color.name() != "RED") begin
	 $display("FAILED -- color.name() != RED");
	 $finish;
      end
      if (color.first() != RED) begin
	 $display("FAILED -- color.first() != RED");
	 $finish;
      end

      while (color != color.last()) begin
	 color = color.next();
	 $display("Next color is %s (%0d)", color.name(), color);
      end

      $display("Last color is %s (%0d).", color.name(), color);
      if (color.name() != "VIOLET") begin
	 $display("FAILED -- color.name() != VIOLET");
	 $finish;
      end
      if (color.last() != VIOLET) begin
	 $display("FAILED -- color.last() != VIOLET");
	 $finish;
      end

      $display("PASSED");
      $finish(0);
   end

endmodule // main
