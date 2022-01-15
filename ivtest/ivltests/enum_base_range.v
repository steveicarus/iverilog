module test;

   // Test declaring the enum as a 3-bit logic.
   enum reg   [2:0] { rstate[8] } reg_enum;
   enum bit   [2:0] { bstate[8] } bit_enum;
   enum logic [2:0] { lstate[8] } log_enum;

   initial begin
      if ($bits(reg_enum) != 3) begin
	 $display("FAILED -- $bits(reg_enum) == %0d", $bits(reg_enum));
	 $finish;
      end
      if ($bits(bit_enum) != 3) begin
	 $display("FAILED -- $bits(bit_enum) == %0d", $bits(bit_enum));
	 $finish;
      end
      if ($bits(log_enum) != 3) begin
	 $display("FAILED -- $bits(log_enum) == %0d", $bits(log_enum));
	 $finish;
      end

      if ($bits(rstate0) != 3) begin
	 $display("FAILED -- $bits(rstate0) == %0d", $bits(rstate0));
	 $finish;
      end
      if ($bits(bstate0) != 3) begin
	 $display("FAILED -- $bits(bstate0) == %0d", $bits(bstate0));
	 $finish;
      end
      if ($bits(lstate0) != 3) begin
	 $display("FAILED -- $bits(lstate0) == %0d", $bits(lstate0));
	 $finish;
      end

      if (rstate0 !== 3'b000 || bstate0 !== 3'b000 || lstate0 !== 3'b000) begin
	 $display("FAILED -- rstate0 == %b", rstate0);
	 $finish;
      end

      if (rstate4 !== 3'b100 || bstate4 !== 3'b100 || lstate4 !== 3'b100) begin
	 $display("FAILED -- rstate4 == %b", rstate4);
	 $finish;
      end

      if (rstate7 !== 3'b111 || bstate7 !== 3'b111 || lstate7 !== 3'b111) begin
	 $display("FAILED -- rstate7 == %b", rstate7);
	 $finish;
      end

      $display ("PASSED");
   end
endmodule // test
