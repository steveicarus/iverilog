module t();

reg [1:0] f;
reg [1:0] oszok;
wire [2:0] vosz1;

genvar i;

generate
	for(i = 0; i < 4; i = i + 1) begin : reg_tomb_gen
		wire [2:0] vosz1;
		assign vosz1 = i - f + oszok + 1;
		initial begin
			#1;
			if(!i && vosz1 !== 0) begin
			   $display("FAIL -- i=%b, f=%b, oszok=%b, vosz1=%b",
				    i, f, oszok, vosz1);
			   $finish;
			end
		end
	end
endgenerate

initial
begin
   f = 3;
   oszok = 2;
   #2 $display("PASSED");
end
endmodule
