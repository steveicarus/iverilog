// This program is based on pr2138979. In particular, the signed
// expressions are sign-extended before the '|' is evaluated. This
// behavior is verified by modelsim and ncverilog. (It appears that
// gplcver gets this wrong.)

module main;

   reg   [7:0] a, b;
   wire [15:0] y;
   reg [15:0]  z;

   // Use $signed() to sign extend operands before logic OR
   //   - Note that Icarus Verilog is not sign extending as expected
   assign y = $signed(a) | $signed(b);

   initial begin
      a = 8'h55;
      b = 8'haa;
      z = $signed(a) | $signed(b);

      #1 if (y !== 16'hff_ff || y !== z) begin
	 $display("FAILED -- a=%h, b=%h, y=%h, z=%h", a, b, y, z);
	 $finish;
      end

      a = 8'haa;
      b = 8'h55;
      z = $signed(a) | $signed(b);

      #1 if (y !== 16'hff_ff || y !== z) begin
	 $display("FAILED -- a=%h, b=%h, y=%h, z=%h", a, b, y, z);
	 $finish;
      end

      a = 8'h7f;
      b = 8'h00;
      z = $signed(a) | $signed(b);

      #1 if (y !== 16'h00_7f || y !== z) begin
	 $display("FAILED -- a=%h, b=%h, y=%h, z=%h", a, b, y, z);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule
