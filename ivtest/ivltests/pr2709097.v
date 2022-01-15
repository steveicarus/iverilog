module top;
    reg [31:0] mem[0:0];

    initial begin
        $readmemh( "ivltests/pr2709097.hex", mem );
//	$display("mem[0] = %d", mem[0]);
	if (mem[0] !== 10) $display("FAILED");
	else $display("PASSED");
    end
endmodule
