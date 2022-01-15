module test;

	logic b;
	logic [9:0] b10;
	logic signed bs;
    logic unsigned bu;
	logic signed [6:0] bs7;
    logic unsigned [5:0] bu6;

	initial
	begin
       b = 1;
	   b10 = 100;
	   bs = 0;
	   bu = 1;
	   bs7 = -17;
	   bu6 = 21;
	   #1;
	   if (b * b10 !== 100) begin
	      $display ("FAILED 1");
          $finish;
	   end
	   if (bs * b10 !== 0) begin
	      $display ("FAILED 2" );
          $finish;
	   end
	   if (bu * b10 !== 100) begin
	      $display ("FAILED 3");
          $finish;
	   end
	   if (bs7 * 1 !== -17) begin
	      $display ("FAILED 4");
          $finish;
	   end
	   if (bu6 * b !== 21) begin
	      $display ("FAILED 5");
          $finish;
	   end
	   #1;
	   bu6 = 6'bx1100z;
	   if (bu6 * 1'b1 !== 6'bxxxxxx) begin
	      $display ("FAILED 6");
          $finish;
	   end
	   $display ("PASSED");
	end

endmodule
