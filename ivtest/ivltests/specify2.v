module main;

   wire dst;
   reg	src;

   spec_buf dut(dst, src);

   initial begin
      src = 0;
      #10 if (dst !== 0) begin
	 $display("FAILED -- setup failed: src=%b, dst=%b", src, dst);
	 $finish;
      end

      src = 1;

      #5 if (dst !== 0) begin
	 $display("FAILED -- dst changed too fast. src=%b, dst=%b", src, dst);
	 $finish;
      end

      #5 if (dst !== 1) begin
	 $display("FAILED -- dst failed to change. src=%b, dst=%b", src, dst);
	 $finish;
      end

      src = 0;

      #5 if (dst !== 1) begin
	 $display("FAILED -- dst changed too fast. src=%b, dst=%b", src, dst);
	 $finish;
      end

      #5 if (dst !== 0) begin
	 $display("FAILED -- dst failed to change. src=%b, dst=%b", src, dst);
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main

module spec_buf(output wire O, input wire I);

   buf (O, I);

   specify
      (I => O) = (7);
   endspecify

endmodule // sec_buf
