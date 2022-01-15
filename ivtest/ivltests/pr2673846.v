module top;
  reg [39:0] x, x_inv_bug, temp_x;
  integer i, j;

   wire [39:0] res2 = {40 {1'd1}} / x;

  initial begin
// Using this assignment instead of the procedural assignment below
// will work with out asserting.
//    x = 1;
//    assign x_inv_bug = {40 {1'd1}} / x;

    temp_x = 2**31;
    for (i=30 ; i < 38; i=i+1)  begin
      temp_x = temp_x << 1;
      for (j=0 ; j<3; j=j+1) begin
        x = temp_x + (j-1);
        $display(" // i,j,temp_x,x => %2d,%1d,%d,%h", i, j, temp_x, x);
        // The following statement is asserting and it looks to be a
        // problem in the division algorithm. This specific case is
        // likely only a 32 bit problem, but by scaling I'm sure this
        // could be made to trigger on a 64 bit machine.
        x_inv_bug = {40 {1'd1}} / x;
	 #1 $display("x_inv_bug=%h, res2=%h", x_inv_bug, res2);
	 if (x_inv_bug !== res2) begin
	    $display("FAILED");
	    $finish;
	 end
      end
    end // for (i=30 ; i < 38; i=i+1)
    $display("PASSED");
  end
endmodule
