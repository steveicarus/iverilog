module main;

   typedef struct packed {
      logic [3:0] high;
      logic [3:0] low;
   } word;

   typedef word [1:0] dword;

   wire		  dword foo;
   int		  idx;

   assign foo[0].low = 1;
   assign foo[0].high = 2;
   assign foo[1].low = 3;
   assign foo[1].high = 4;

   initial begin
      #1 $display("foo = %h", foo);

      if (foo !== 16'h4321) begin
	 $display("FAILED -- foo=%h", foo);
	 $finish;
      end

      $display("foo[0] = %h", foo[0]);
      if (foo[0] !== 8'h21) begin
	 $display("FAILED -- foo[0]=%h", foo[0]);
	 $finish;
      end

      $display("foo[1] = %h", foo[1]);
      if (foo[1] !== 8'h43) begin
	 $display("FAILED -- foo[1]=%h", foo[1]);
	 $finish;
      end

      $display("foo[0].low  = %h", foo[0].low);
      if (foo[0].low !== 4'h1) begin
         $display("FAILED -- foo[0].low=%h", foo[0].low);
	 $finish;
      end

      $display("foo[0].high = %h", foo[0].high);
      if (foo[0].high !== 4'h2) begin
         $display("FAILED -- foo[0].high=%h", foo[0].high);
	 $finish;
      end

      $display("foo[1].low  = %h", foo[1].low);
      if (foo[1].low !== 4'h3) begin
         $display("FAILED -- foo[1].low=%h", foo[1].low);
	 $finish;
      end

      $display("foo[1].high = %h", foo[1].high);
      if (foo[1].high !== 4'h4) begin
         $display("FAILED -- foo[1].high=%h", foo[1].high);
	 $finish;
      end

      idx = 0;
      $display("foo[idx=0].low = %h", foo[idx].low);
      if (foo[idx].low !== 4'h1) begin
         $display("FAILED -- foo[0].low=%h", foo[idx].low);
	 $finish;
      end

      idx = 1;
      $display("foo[idx=1].high = %h", foo[idx].high);
      if (foo[idx].high !== 8'h4) begin
	 $display("FAILED -- foo[1].high=%h", foo[idx].high);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
