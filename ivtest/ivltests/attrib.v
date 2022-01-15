// This test program is mostly about the parser parsing the attribute
// attached to the main.dut.Q reg below.

module main;

   reg CK;
   always begin
      #10 CK = 0;
      #10 CK = 1;
   end

   reg [3:0] D;
   wire [3:0] Q;
   test dut (.Q(Q), .D(D), .CK(CK));

   initial begin
      D = 0;
      @(posedge CK) #1 $display("Q=%b, D=%b", Q, D);
      if (Q !== D) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main

module test (Q, D, CK);

   output [3:0] Q;
   input [3:0]	D;
   input	CK;

   (* REGISTER_DUPLICATION = "no" *)
   reg [3:0]	Q;
   always @(posedge CK)
     Q <= D;

endmodule // test
