/*
 * The x in foo and bar should always have the same value.
 */
module foo();
  wire x;
  reg clk;
  reg [7:0] counter;

  always #5 clk <= ~clk;

  assign x = 0;

  initial begin
    clk  = 0;
    counter = 0;
     # 2600 $display("PASSED");
            $finish;
  end

   always @(negedge clk)
      if (x !== u_bar.x) begin
	 $display("FAILED -- x != u_bar.x");
	 $finish;
      end

  always @(posedge clk) begin
    counter <= counter + 1;

    if (counter == 32)
      force x = 0;
    else if (counter == 64)
      force x = 1;
    else if (counter == 96)
      force x = 0;
    else if (counter == 128)
      release x;

    $display("[foo %d] x = %d",counter, x);
  end

  bar u_bar( .clk(clk), .x(x));

endmodule

module bar(clk, x);
  input clk;
  input x;

  reg [7:0] counter;

  initial counter = 0;

  always @(posedge clk) begin
    counter <= counter + 1;

    $display("[bar %d] x = %d",counter, x);
  end
endmodule
