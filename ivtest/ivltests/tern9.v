/* tern9.v
 */
module main;

   reg flag;
   reg val;
   wire test1 = flag? val : 1'bx;
   wire test2 = flag? 1'b0 : 1'bx;
   wire test3 = flag? 1'bx : val;

   initial begin
      flag = 1;
      val = 0;

      #1 if (test1 !== 1'b0) begin
	 $display("FAILED -- flag=%b, val=%b, test1=%b", flag, val, test1);
	 $finish;
      end

      if (test2 !== 1'b0) begin
	 $display("FAILED -- flag=%b, test2=%b", flag, test2);
	 $finish;
      end

      if (test3 !== 1'bx) begin
	 $display("FAILED -- flag=%b, test3=%b", flag, test3);
	 $finish;
      end

      val = 1;

      #1 if (test1 !== 1'b1) begin
	 $display("FAILED -- flag=%b, val=%b, test1=%b", flag, val, test1);
	 $finish;
      end

      val = 1'bx;

      #1 if (test1 !== 1'bx) begin
	 $display("FAILED -- flag=%b, val=%b, test1=%b", flag, val, test1);
	 $finish;
      end

      val = 1'bz;

      #1 if (test1 !== 1'bz) begin
	 $display("FAILED -- flag=%b, val=%b, test1=%b", flag, val, test1);
	 $finish;
      end

      flag = 0;
      val = 0;

      #1 if (test1 !== 1'bx) begin
	 $display("FAILED -- flag=%b, val=%b, test1=%b", flag, val, test1);
	 $finish;
      end

      if (test2 !== 1'bx) begin
	 $display("FAILED -- flag=%b, test2=%b", flag, test2);
	 $finish;
      end

      if (test3 !== 1'b0) begin
	 $display("FAILED -- flag=%b, test3=%b", flag, test3);
	 $finish;
      end

      val = 1;

      #1 if (test1 !== 1'bx) begin
	 $display("FAILED -- flag=%b, val=%b, test1=%b", flag, val, test1);
	 $finish;
      end

      if (test3 !== 1'b1) begin
	 $display("FAILED -- flag=%b, test3=%b", flag, test3);
	 $finish;
      end

      val = 1'bx;

      #1 if (test3 !== 1'bx) begin
	 $display("FAILED -- flag=%b, val=%b, test3=%b", flag, val, test3);
	 $finish;
      end

      val = 1'bz;

      #1 if (test3 !== 1'bz) begin
	 $display("FAILED -- flag=%b, val=%b, test3=%b", flag, val, test3);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule
