
module DFF
  (output reg Q0,
   output reg [1:0] Q1,
   input wire D0,
   input wire [1:0] D1,
   input wire CLK,
   input wire RST
   /* */);

   always @(posedge CLK or posedge RST)
     if (RST) begin
	Q0 <= 0;
	Q1 <= 0;
     end else begin
	Q0 <= D0;
	Q1 <= D1;
     end

endmodule // dut

module main;

   wire       q0;
   wire [1:0] q1;
   reg	      d0, clk, rst;
   reg [1:0]  d1;

   DFF dut (.Q0(q0), .Q1(q1), .D0(d0), .D1(d1), .CLK(clk), .RST(rst));

   initial begin
      clk <= 1;
      d0 <= 0;
      d1 <= 2;

      #1 rst <= 1;
      #1 if (q0 !== 1'b0 || q1 !== 1'b0) begin
	 $display("FAILED -- RST=%b, Q0=%b, Q1=%b", rst, q0, q1);
	 $finish;
      end

      #1 rst <= 0;
      #1 if (q0 !== 1'b0 || q1 !== 1'b0) begin
	 $display("FAILED -- RST=%b, Q0=%b, Q1=%b", rst, q0, q1);
	 $finish;
      end

      #1 clk <= 0;
      #1 clk <= 1;
      #1 if (q0 !== d0 || q1 !== d1) begin
	 $display("FAILED -- Q0=%b Q1=%b, D0=%b D1=%b", q0, q1, d0, d1);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
