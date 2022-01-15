/*
 * This is a simple test of a generate loop.
 */
module main;

   parameter       SIZE = 4;
   wire [SIZE:0]   cin;
   reg [SIZE-1:0]  a, b;
   wire [SIZE-1:0] q;

   // This generates a ripple adder by using a generate loop to
   // instantiate an array of half-adders.

   genvar i;

   assign	 cin[0] = 0;
   for (i=0 ; i<SIZE ;  i = i+1) begin : slice
      wire [1:0] sum      = a[i] + b[i] + cin[i];
      assign     q[i]     = sum[0];
      assign     cin[i+1] = sum[1];
   end

   wire [SIZE:0] total = {cin[SIZE], q};

   // Test the ripple adder by finding all the possible sums.
   reg [2*SIZE:0] idx;
   initial begin
      for (idx = 0 ;  idx < (1<<(2*SIZE)) ;  idx = idx+1) begin
	 a = idx[  SIZE-1:   0];
	 b = idx[2*SIZE-1:SIZE];
	 #1 if (total !== a+b) begin
	    $display("FAILED -- total=%b, a=%b, b=%b", total, a, b);
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule
