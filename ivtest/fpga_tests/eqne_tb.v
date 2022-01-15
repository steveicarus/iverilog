module main;

   wire eq1, eq2, eq5;
   wire ne1, ne2, ne5;

   reg [7:0] x, y;

   eqne dut(.eq1(eq1), .eq2(eq2), .eq5(eq5),
	    .ne1(ne1), .ne2(ne2), .ne5(ne5),
	    .x(x), .y(y));

   initial begin
      for (x = 0 ;  x < 'h20 ;  x = x+1)
	for (y = 0 ;  y < 'h20 ;  y = y+1) begin
	   #1 $display("x=%h, y=%h: ", x, y,
		       "eq1=%b, eq2=%b, eq5=%b, ", eq1, eq2, eq5,
		       "ne1=%b, ne2=%b, ne5=%b", ne1, ne2, ne5);
	   if (eq1 !== (x[0] == y[0])) begin
	      $display("FAILED");
	      $finish;
	   end

	   if (eq2 !== (x[1:0] == y[1:0])) begin
	      $display("FAILED");
	      $finish;
	   end

	   if (eq5 !== (x[4:0] == y[4:0])) begin
	      $display("FAILED");
	      $finish;
	   end

	   if (ne1 !== (x[0] != y[0])) begin
	      $display("FAILED");
	      $finish;
	   end

	   if (ne2 !== (x[1:0] != y[1:0])) begin
	      $display("FAILED");
	      $finish;
	   end

	   if (ne5 !== (x[4:0] != y[4:0])) begin
	      $display("FAILED");
	      $finish;
	   end
	end

      $display("PASSED");
   end

endmodule // main
