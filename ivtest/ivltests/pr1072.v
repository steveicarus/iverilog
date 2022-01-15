/*
 * This test program catches the essence of PR#1072
 */
module main;

   parameter WIDTH = 4;

   wire [19:0] foo = { 1<<WIDTH {1'b1}};
   reg [19:0]  bar;

   initial begin
      #1 bar = { 1<<WIDTH {1'b1}};

      if (foo !== 20'h0ffff) begin
	 $display("FAILED -- foo = %b", foo);
	 $finish;
      end

      if (bar !== 20'h0ffff) begin
	 $display("FAILED -- bar = %b", bar);
	 $finish;
      end

      if (foo !== bar) begin
	 $display("FAILED -- foo !== bar (%h !== %h)", foo, bar);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
