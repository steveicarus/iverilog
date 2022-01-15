/* real9.v
 * This tests comparison of a real variable with integer constants.
 */
module main;

   real value;
   parameter param = 2;

   initial begin
      value = 3.0;

      if (value < param) begin
	 $display("FAILED -- %f < %d", value, param);
	 $finish;
      end

      if (value < 2) begin
	 $display("FAILED -- %f < 2", value);
	 $finish;
      end

      if (value <= param) begin
	 $display("FAILED -- %f <= %d", value, param);
	 $finish;
      end

      if (value <= 2) begin
	 $display("FAILED -- %f <= 2", value);
	 $finish;
      end

      if (value == param) begin
	 $display("FAILED -- %f == %d", value, param);
	 $finish;
      end

      if (value == 2) begin
	 $display("FAILED -- %f == 2", value);
	 $finish;
      end

      if (param >= value) begin
	 $display("FAILED -- %d >= %f", param, value);
	 $finish;
      end

      if (2 >= value) begin
	 $display("FAILED -- 2 >= %f", value);
	 $finish;
      end

      value = 2.0;

      if (value < param) begin
	 $display("FAILED -- %f < %d", value, param);
	 $finish;
      end

      if (value < 2) begin
	 $display("FAILED -- %f < 2", value);
	 $finish;
      end

      if (value != param) begin
	 $display("FAILED -- %f != %d", value, param);
	 $finish;
      end

      if (value != 2) begin
	 $display("FAILED -- %f != 2", value);
	 $finish;
      end

      if (value > param) begin
	 $display("FAILED -- %f > %d", value, param);
	 $finish;
      end

      if (value > 2) begin
	 $display("FAILED -- %f > 2", value);
	 $finish;
      end

      value = 1.6;

      if (value == param) begin
	 $display("FAILED -- %f == %d", value, param);
	 $finish;
      end

      if (value == 2) begin
	 $display("FAILED -- %f == 2", value);
	 $finish;
      end

      if (value >= param) begin
	 $display("FAILED -- %f >= %d", value, param);
	 $finish;
      end

      if (value >= 2) begin
	 $display("FAILED -- %f >= 2", value);
	 $finish;
      end

      if (value > param) begin
	 $display("FAILED -- %f > %d", value, param);
	 $finish;
      end

      if (value > 2) begin
	 $display("FAILED -- %f > 2", value);
	 $finish;
      end

      $display("PASSED");
   end // initial begin


endmodule
