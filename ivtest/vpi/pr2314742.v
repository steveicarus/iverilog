module top;

   reg [7:0] a;
   reg [7:0] b;
   reg [7:0] c;

   integer   retcode;


   initial
      begin
         #0; // avoid T0 race
	 a = 0;
	 b = 0;
	 c = 0;
	 /* Use VPI to set values on these registers */
	 retcode = $example(a, b, c);
      end

   always @(a)
     $display("%t The value of A is: %b", $time, a);

   always @(b)
     $display("%t The value of B is: %b", $time, b);

   always @(c)
     $display("%t The value of C is: %b", $time, c);

endmodule // top
