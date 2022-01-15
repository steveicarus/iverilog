/*
 * This test runs the random number generator to make sure
 * it follows the standard algorithm. It is based on the bug
 * report from PR#995.
 */

module pr995 ();
   integer seed;
   integer i;
   integer result;

   initial begin
      seed = 1;
      $display ("Start sequence: seed=%h", seed);

      result=$random(seed);
      $display ("seed=%h result=%h", seed, result);

      for (i=0; i<30; i=i+1) begin
	 result=$random(seed);
	 $display ("seed=%h result=%h", seed, result);
      end

      seed = 2;
      $display ("Start sequence: seed=%h", seed);

      result=$random(seed);
      $display ("seed=%h result=%h", seed, result);

      for (i=0; i<30; i=i+1) begin
	 result=$random(seed);
	 $display ("seed=%h result=%h", seed, result);
      end

      seed = 1;
      $display ("Start sequence: seed=%h", seed);

      result=$random(seed);
      $display ("seed=%h result=%h", seed, result);

      for (i=0; i<30; i=i+1) begin
	 result=$random(seed);
	 $display ("seed=%h result=%h", seed, result);
      end
   end
endmodule // rand
