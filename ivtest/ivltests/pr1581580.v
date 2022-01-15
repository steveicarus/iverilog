module test;

   parameter SIZE = 2;

   reg [SIZE-1:0] d ;	// data in
   reg		  c ;	// latch control
   wire [SIZE-1:0] q ;	// output

   unit_latch u_lat[SIZE-1:0] (.Q(q), .G(c), .D(d));

   initial begin
      d = 0;
      c = 1;
      #1 if (q !== 2'b00) begin
	 $display("FAILED -- Initial load failed.");
	 $finish;
      end

      d = 2'b01;

      #1 if (q !== 2'b01) begin
	 $display("FAILED -- Latch follow failed.");
	 $finish;
      end

      c = 0;
      #1 d = 2'b10;
      #1 if (q !== 2'b01) begin
	 $display("FAILED -- Latch hold failed.");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule

module unit_latch(output reg Q, input wire D, input wire G);

   always @*
     if (G) Q = D;

endmodule // unit_latch
