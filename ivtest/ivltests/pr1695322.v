module test ();

   wire [5:0] a [0:2];

   b b(.a(a[0]));

   initial begin
      #1 if (a[0] !== 5) begin
	 $display("FAILED -- a[0] == %d", a[0]);
	 $finish;
      end

      if (a[1] !== 6'bzzzzzz) begin
	 $display("FAILED -- a[1] == %h", a[1]);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule

module b (output wire [5:0] a);

   assign a = 5;

endmodule
