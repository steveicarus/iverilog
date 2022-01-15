//
// Verifies that the PRNG seed streams are unique, well trys anyway.
//
module test;
    reg [31:0] rtn;
    reg [31:0] pseed, seed1, seed2;
    reg [31:0] mem1[3:0], mem2[3:0];
    integer i;

    initial begin
	seed1 = 32'hcafe_babe;
	seed2 = 32'hdead_beef;

	// Isolated stream
	for (i = 0; i < 4; i = i + 1) begin
	    mem1[i] = $random(seed1);
	end

	// Pull from multiple streams
	seed1 = 32'hcafe_babe;
	seed2 = 32'hdead_beef;
	for (i = 0; i < 4; i = i + 1) begin
	    mem2[i] = $random(seed1);
	    // pull more values from other pools
	    rtn = $random(seed2);
	    rtn = $random;
	end

	// Verify the seed1 streams match
	for (i = 0; i < 4; i = i + 1) begin
	    if (mem1[i] != mem2[i]) begin
		$display("FAILED %0d: %x != %x", i, mem1[i], mem2[i]);
		$finish;
	    end
	end

	$display("PASSED");
	$finish;
    end
endmodule
