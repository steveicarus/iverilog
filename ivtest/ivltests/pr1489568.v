// pr1489568

module bug();

  reg  [15 : 0] in;
  reg  sel;
  wire [31 : 0] result = { 16'd0, sel ? -in : in };

   initial begin
      in = 2;
      sel = 0;
      #1 if (result !== 32'h0000_0002) begin
	 $display("FAILED -- sel=%b, in=%h, result=%h", sel, in, result);
	 $finish;
      end

      sel = 1;
      #1 if (result !== 32'h0000_fffe) begin
	 $display("FAILED -- sel=%b, in=%h, result=%h", sel, in, result);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
