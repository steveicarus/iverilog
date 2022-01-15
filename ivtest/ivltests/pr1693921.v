module test;

   reg [3:0] foo;
   reg [3:0] shift;

   wire [3:0] rs = foo >> shift;
   wire [3:0] ls = foo << shift;

   wire       tr = 4'b0100 > (foo >> shift);

   initial begin
      foo = 4'b1001;
      shift = 0;

      #1 if (rs !== 4'b1001 || ls !== 4'b1001) begin
	 $display("FAILED  -- shift=%d, rs=%b, ls=%b", shift, rs, ls);
	 $finish;
      end

      shift = 1;

      #1 if (rs !== 4'b0100 || ls !== 4'b0010 || tr !== 0) begin
	 $display("FAILED  -- shift=%d, rs=%b, ls=%b", shift, rs, ls);
	 $finish;
      end

      shift = 2;

      #1 if (rs !== 4'b0010 || ls !== 4'b0100 || tr !== 1) begin
	 $display("FAILED  -- shift=%d, rs=%b, ls=%b", shift, rs, ls);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
