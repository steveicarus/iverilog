module test;

   reg [31:0] src;
   wire [7:0] tmp;

   subbuf U1 (.out(tmp), .in(src));

   wire [31:0] dst = {24'h00_00_00, tmp};

   initial begin
      src = 32'h11_22_33_44;
      #1 if (dst !== 32'h00_00_00_bb) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule

module subbuf (output [31:0] out, input[31:0]in);
   assign out = ~in;
endmodule // subbuf
