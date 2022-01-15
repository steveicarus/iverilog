module test ();

   wire [2:0] d [0:2];
   reg [2:0]  src[0:2];

   genvar i;
   for (i = 0 ;  i < 3 ; i = i+1)
     assign d[i] = src[i];

   integer idx;
   initial begin
      for (idx = 0 ;  idx < 3 ;  idx  = idx+1)
	src[idx] = idx;

      #1 for (idx = 0 ;  idx < 3 ;  idx = idx+1)
	if (d[idx] !== idx) begin
	   $display("FAILED -- d[%0d] = %b", idx, d[idx]);
	   $finish;
	end

      $display("PASSED");
   end // initial begin

endmodule
