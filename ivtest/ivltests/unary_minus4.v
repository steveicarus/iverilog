module bug();

  reg [15 : 0] in;
  reg sel;
  wire [31 : 0] result = { 16'd0, sel ? -in : in };

  initial begin
     in = 100;
     sel = 0;
     #1 if (result !== 32'h0000_0064) begin
	$display("FAILED -- result=%h, sel=%b, in=%h", result, sel, in);
	$finish;
     end

     sel = 1;
     #1 if (result !== 32'h0000_ff9c) begin
	$display("FAILED == result=%h, sel=%b, in=%h, -in=%h",
		 result, sel, in, -in);
	$finish;
     end

     $display("PASSED");
  end

endmodule
