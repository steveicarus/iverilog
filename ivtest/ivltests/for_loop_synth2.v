
module test
  #(parameter inputs = 8)
   (output reg [inputs-1:0] Q,
    input wire [inputs-1:0] D,
    input wire [inputs-1:0] I);

   // This should synthesize to an unrolled version of the
   // for loop.
   integer		    j;
   always @* begin
      for (j = 0 ; j < inputs ; j += 1) begin
	 if (I[j]) Q[j] = ~D[j];
	 else Q[j] = D[j];
      end
   end

endmodule // test

module main;

   wire [7:0] Q;
   reg [7:0]  D, I;

   test dut (.Q(Q), .D(D), .I(I));

   (* ivl_synthesis_off *)
   initial begin
      D = 0;
      I = 0;
      #1 if (Q !== 8'h00) begin
	 $display("FAILED -- Q=%h, D=%h, I=%h", Q, D, I);
	 $finish;
      end

      D = 'h55;
      I = 'h55;
      #1 if (Q !== 8'h00) begin
	 $display("FAILED -- Q=%h, D=%h, I=%h", Q, D, I);
	 $finish;
      end

      D = 'h55;
      I = 'haa;
      #1 if (Q !== 8'hff) begin
	 $display("FAILED -- Q=%h, D=%h, I=%h", Q, D, I);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule // main
