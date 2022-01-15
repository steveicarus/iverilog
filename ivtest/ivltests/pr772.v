/*
 * Based on bug report pr772.
 */

module err ();

parameter kuku = "AAAAA";


reg reset_b,clk;
initial begin
      reset_b  = 0;
      repeat (10) @(posedge clk);
      #1 reset_b = 1;
end

initial begin
      clk = 1'b1;
      #3 forever #10 clk=~clk;
end






always @(posedge clk or negedge reset_b)
  if (!reset_b) begin
  end
  else begin
    if ((kuku=="RRRRR") || (kuku=="AAAAA") || (kuku=="BBBBB"))
	$display("PASSED");
    else $display("FAILED");
    $finish;
  end


endmodule
