// Extracted from PR#224

module test;

   reg        clk;
   reg [3:0]  ack;

   task first;
      input [1:0]  p;
      begin
         @(posedge clk); $display("got posedge clk");
`ifdef LINE_A
//A: line below compiles under XL/NC - iverilog complains
	 @(posedge ack[p]); $display("got posedge ack[p]");
`else
//B: line below core dumps under vvp - OK under vvm
         @(posedge ack); $display("got posedge ack");
`endif
         @(posedge clk); $display("got posedge clk");
	 $display("PASSED");
	 $finish;
      end
   endtask

   initial #5 first(1);

   initial
     begin
	ack <= 0; clk <= 0;
	#10 clk <= 1;
	#10 ack <= 3; clk <= 0;
	#10 clk <= 1;
	#10 $display("FAILED");
	$finish;
     end

endmodule // test
