//
// Test the number format insanity
//
module test;

    integer err;
    reg [31:0] i;

    // Ugly specification
    initial begin
	i = 659;
	i = 'h 837FF;
	i = 'o7460;
	i = 4'b1001;
	i = 5 'D 3;
	i = 3'b01x;
	i = 12'hx;
	i = 16'hz;
	i = -8 'd 6;
	i = 4 'shf;
	i = -4 'sd15;
    end

    //always @(i) $display("%0t:\ti = %d", $time, i);

    // Potential ambiguities
    initial begin
	err = 0;

       i = # 9  1'd0;

	i = # 9_7 'D 3;
	#100;
	if (i != 'd3) begin
	    $display("'d3 != %0d", i);
	    err = 1;
	end

	i = # 1 'h 123;
	#100;
	if (i != 'h123) begin
	    $display("'h123 != %0h", i);
	    err = 1;
	end

	i = #(5 'D 3) 'D 3;
	#100;
	if (i != 'd3) begin
	    $display("'d3 != %0d", i);
	    err = 1;
	end

	i = # 93 'h 837FF;
	#100;
	if (i != 'h837ff) begin
	    $display("'h837ff != %0h", i);
	    err = 1;
	end

	i = # 33 20 'h 837ff - 1;
	#100;
	if (i != 'h837fe) begin
	    $display("'h837fe != %0h", i);
	    err = 1;
	end

	i = # 69  - 20 'd 255 + 20'd1;
	#100;
	if (i[19:0] != 20'hf_ff_02) begin
	    $display("- 'd254 != %0d (%h)", i, i);
	    err = 1;
	end

	i = #(27 - 20)'d 254 + 1;
	#100;
	if (i != 10'd255) begin
	    $display("'d255 != %0d", i);
	    err = 1;
	end

	i = # 97.4 'h abcd;
	#100;
	if (i != 'habcd) begin
	    $display("'abcd != %0h", i);
	    err = 1;
	end


	if (err)
	    $display("FAILED");
	else
	    $display("PASSED");
    end
endmodule
