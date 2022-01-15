`timescale 10ns/1ps

module main;
   logic [1:0] counter = 2'b00;
   logic       clk = 1'b0;

   initial forever #1 clk <= ~clk;

   always @(posedge clk) begin
      counter <= counter + 2'd1;

      unique case (counter)
      2'd0: $display("case 0");
      2'd1: $display("case 1");
      2'd3: $display("case 3");
      endcase // priority case (counter)

      if (counter == 2'd3) begin
         $display("PASSED");
         $finish(0);
      end
   end
endmodule
