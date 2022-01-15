module main;

   struct packed {
      logic [1:0][7:0] p;
   } val;

   logic [1:0][7:0]    arr;

   initial begin
      arr[0] = 'haa;
      arr[1] = 'h55;
      $display("arr = %h", arr);
      if (arr !== 'h55_aa) begin
	 $display("FAILED");
	 $finish;
      end

      if (arr[0] !== 'haa) begin
	 $display("FAILED -- arr[0]=%h", arr[0]);
	 $finish;
      end
      if (arr[1] !== 'h55) begin
	 $display("FAILED -- arr[1]=%h", arr[1]);
	 $finish;
      end

      val.p[0] = 'haa;
      val.p[1] = 'h55;
      $display("val.p = %h", val.p);
      $display("val = %h", val);
      if (val !== 'h55_aa) begin
	 $display("FAILED");
	 $finish;
      end
      if (val.p !== 'h55_aa) begin
	 $display("FAILED");
	 $finish;
      end

      if (val.p[0] !== 'haa) begin
	 $display("FAILED -- val.p[0]=%h", val.p[0]);
	 $finish;
      end
      if (val.p[1] !== 'h55) begin
	 $display("FAILED -- val.p[1]=%h", val.p[1]);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
