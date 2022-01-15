module test;

	bit b;
	bit [9:0] b10;
	bit signed bs;
    bit unsigned bu;
	bit signed [6:0] bs7;
    bit unsigned [5:0] bu6;

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
	   $display ("PASSED");
	end

endmodule
