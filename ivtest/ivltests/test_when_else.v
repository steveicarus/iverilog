module main;

   reg [1:0] src;
   wire [3:0] dst, dst2, dst3;

   foo_entity dut (.data_o(dst), .data_o2(dst2), .data_o3(dst3), .data_i(src));

   initial begin
      src = 2'b00;
      #1 if (dst != 4'b0001 || dst2 != 4'bxxxx || dst3 != 4'bxxx) begin
	 $display("FAILED");
	 $finish;
      end

      src = 2'b01;
      #1 if (dst != 4'b0010 || dst2 != 4'b0101 || dst3 != 4'b0011) begin
	 $display("FAILED");
	 $finish;
      end

      src = 2'b10;
      #1 if (dst != 4'b0100 || dst2 != 4'b0101 || dst3 != 4'b1100) begin
	 $display("FAILED");
	 $finish;
      end

      src = 2'b11;
      #1 if (dst != 4'b1000 || dst2 != 4'b0101 || dst3 != 4'b1100) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
